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
 * MP4 player library
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bmp4_parser.h"
#include "bmp4_player.h"
#include "bmp4_track.h"
#include "biobits.h"

#include "blst_squeue.h"

BDBG_MODULE(bmp4_player);
#define BDBG_MSG_TRACE(x) BDBG_MSG(x)

BDBG_OBJECT_ID(bmp4_player_t);

typedef struct b_mp4_player_handler {
	bmp4_parser_handler handler; /* must be first member */
	bmp4_player_t player;
} b_mp4_player_handler;


typedef struct b_mp4_movie {
	unsigned ntracks;
	BLST_SQ_HEAD(b_mp4_tracks, b_mp4_track) tracks;
	bmp4_movieheaderbox movieheader;
	bmp4_filetypebox filetype;
	bool error;
	bool valid;
	bool movieheader_valid;
	bool filetype_valid;
} b_mp4_movie;

typedef struct b_mp4_active_box {
    uint32_t type;
    off_t start;
    uint64_t len;
    size_t header_len;
    uint64_t next; /* next reading point (inside the segment) */
} b_mp4_active_box;

typedef struct b_mp4_fragment_track_state {
    struct {
        uint64_t run_offset;
        uint64_t run_start_time;
    } next, current;
    struct {
        unsigned index_entry; /* current index entry, updated by seek and trick play */
        bool end_of_index; /* reached end of index, updated by trick play */
    } trick; /* these is _not_ cleared by b_mp4_fragment_track_clear */
} b_mp4_fragment_track_state;

typedef struct bmp4_track_fragment_random_access_header {
    uint32_t track_ID;
    unsigned length_size_of_traf_num;
    unsigned length_size_of_trun_num;
    unsigned length_size_of_sample_num;
    uint32_t number_of_entry;
} bmp4_track_fragment_random_access_header;

typedef struct b_mp4_fragment_track {
    bfile_cached_segment cache;
    bmp4_track_fragment_header track_fragment_header;
    bmp4_track_fragment_run_header run_header;
    bmp4_track_fragment_run_state run_state;
    bmp4_track_fragment_run_sample next_sample;
    bool next_sample_valid;
    bool fragment_run_valid;
    bool fragment_header_valid;
    bool movie_fragment_valid;
    bool movie_fragment_data_valid;
    b_mp4_active_box root;
    b_mp4_active_box movie_fragment;
    b_mp4_active_box movie_fragment_data;
    b_mp4_active_box track_fragment;
    b_mp4_active_box track_fragment_run;
    b_mp4_fragment_track_state state;
    struct {
        bool valid;
        uint8_t tfra_version;
        size_t tfra_entry_size;
        bfile_cache_t tfra_cache;
        bmp4_track_fragment_random_access_header tfra_header;
    } index;
} b_mp4_fragment_track;

typedef struct b_mp4_track {
    BLST_SQ_ENTRY(b_mp4_track) next;
    bool trackheader_valid;
    bool mediaheader_valid;
    bool track_extends_valid;
    bool handler_valid;
    bool sample_valid;
    bool next_sample_valid;
    bool fragment_valid;
    bool attached;
    bool bounded_pes;
    uint8_t pes_id;
    bmp4_trackextendsbox track_extends;
    bmp4_track_info info;
    bmp4_sample next_sample;
    bmp4_stream_sample_t stream;
    b_mp4_fragment_track fragment;
    batom_accum_t track_accum; /* accumulator that is used to keep data across different samples */
    /* accumulator */
    void *accum_data; /* pointer to dynamically allocate track data */

    union {
        bmedia_adts_header mp4a;
        struct {
            size_t pcm_packet_size;
        } pcm;
#if 0
        struct {
            uint8_t dqt[4];
            uint8_t dht[4];
            uint8_t sof[4];
            uint8_t sos[4];
        } mjpeg;
#endif
    } codec;
    size_t hdr_len;
    uint8_t hdr_buf[3200];
} b_mp4_track;

#define B_MP4_PLAYER_REW_STACK  30
#define B_MP4_PLAYER_REW_STEP   (60*1000)

typedef struct b_mp4_rew_stack {
    unsigned stack_next;
    bmedia_player_pos position;
    bmp4_sample stack[B_MP4_PLAYER_REW_STACK];
} b_mp4_rew_stack;

typedef struct b_mp4_fragment_player {
    bfile_buffer_t buffer;
    void *buffer_data;
    struct {
        bool valid;
    } index;
} b_mp4_fragment_player;

struct bmp4_player {
    BDBG_OBJECT(bmp4_player_t)
    batom_factory_t factory;
    batom_accum_t accum_src;
    batom_accum_t accum_dest;
    bfile_io_read_t  fd;
    bmedia_player_config config;
    bmedia_time_scale time_scale;
    bmedia_player_stream stream;
    bmedia_player_status status;
    size_t  chunk_size;
    b_mp4_movie movie;
    b_mp4_rew_stack rew_stack;
    b_mp4_track *current_track;
    b_mp4_track *continuation_track;
    bool eof_reached;

    bool fragmented;
    bfile_segment first_movie_fragment;
    b_mp4_fragment_player fragment;

    b_mp4_player_handler filetype_handler;
    b_mp4_player_handler movie_handler;
    b_mp4_player_handler track_handler;
    b_mp4_player_handler media_handler;
    b_mp4_player_handler mediainfo_handler;
    b_mp4_player_handler sampletable_handler;
    b_mp4_player_handler movie_extends_handler;
    uint64_t next_seek;
    uint8_t pes_buf[256];
};


static void
b_mp4_player_atom_free(batom_t atom, void *user)
{
    void *ptr = *(void **)user;
    BDBG_MSG_TRACE(("b_mp4_player_atom_free:%#lx", (unsigned long)atom));
    BSTD_UNUSED(atom);
    BKNI_Free(ptr);
    return;
}

static const batom_user b_mp4_player_atom = {
    b_mp4_player_atom_free,
    sizeof(void **)
};

static void
b_mp4_active_box_init(b_mp4_active_box *box, uint32_t type)
{
    box->next = 0;
    box->type = type;
    box->start = -1;
    box->len = 0;
    return;
}

#if 0
static void
b_mp4_active_box_clone(b_mp4_active_box *box, const b_mp4_active_box *source)
{
    *box = *source;
    return;
}
#endif

static void
b_mp4_active_box_set(b_mp4_active_box *box, off_t start, uint64_t len, size_t header_len)
{
    BDBG_MSG_TRACE(("b_mp4_active_box_set:%#lx " B_MP4_TYPE_FORMAT " %lu:%u", (unsigned long)box, B_MP4_TYPE_ARG(box->type), (unsigned long)start, (unsigned)len));
    box->header_len = header_len;
    box->start = start;
    box->len = len;
    box->next = 0;
    return;
}

static int
b_mp4_active_box_peek(b_mp4_active_box *box, bfile_cached_segment *cache, b_mp4_active_box *child, bool *end_of_data, bmp4_box *mp4_box, size_t *header_size)
{
    size_t box_size;
    uint64_t position;

    *header_size = 0;
    *end_of_data = false;
    BDBG_ASSERT(box->start>=0);
    BDBG_MSG(("b_mp4_active_box_peek:%p " B_MP4_TYPE_FORMAT "(" B_MP4_TYPE_FORMAT ")  %u(%u)", (void *)box, B_MP4_TYPE_ARG(box->type), B_MP4_TYPE_ARG(child->type), (unsigned)box->start, (unsigned)box->len));
    bfile_cached_segment_set(cache, box->start, box->len);
    bfile_cached_segment_seek(cache, box->next);
    /* coverity[check_return] */
    bfile_cached_segment_reserve(cache, BMP4_BOX_MAX_SIZE);
    if(BATOM_IS_EOF(&cache->cursor)) {
        *end_of_data = true;
        return -1;
    }
    box_size = bmp4_parse_box(&cache->cursor, mp4_box);
    *header_size = box_size;
    position = bfile_cached_segment_tell(cache);
    if(box_size==0 || box_size>mp4_box->size) {
        if(position + BMP4_BOX_MAX_SIZE >= box->len) {
            *end_of_data = true;
        }
        return -1;
    }
    BDBG_MSG(("b_mp4_active_box_peek: %#lx:" B_MP4_TYPE_FORMAT " box: " B_MP4_TYPE_FORMAT ":%u (" B_MP4_TYPE_FORMAT ")", (unsigned long)box, B_MP4_TYPE_ARG(box->type), B_MP4_TYPE_ARG(mp4_box->type), (unsigned)mp4_box->size, B_MP4_TYPE_ARG(child->type)));
    if(mp4_box->type==child->type) {
        b_mp4_active_box_set(child, box->start+position, mp4_box->size-box_size, box_size);
    }
    return 0;
}

static int
b_mp4_active_box_next(b_mp4_active_box *box, bfile_cached_segment *cache, b_mp4_active_box *child, bool *end_of_data)
{
    for(;;) {
        int rc;
        bmp4_box mp4_box;
        size_t mp4_header_size;

        rc = b_mp4_active_box_peek(box, cache, child, end_of_data, &mp4_box, &mp4_header_size);
        if(rc<0) {
            return rc;
        }
        box->next += mp4_box.size;
        if(mp4_box.type==child->type) {
            if( !(mp4_box.size >= mp4_header_size && bfile_cached_segment_reserve(cache, mp4_box.size-mp4_header_size))) {
                return -1;
            }
            break;
        }
    }
    return 0;
}


static bmp4_parser_action  
b_mp4_player_movie(bmp4_parser_handler *handler, uint32_t type, batom_t box)
{
	bmp4_player_t player = ((b_mp4_player_handler *)handler)->player;
	bmp4_parser_action action = bmp4_parser_action_none;

	BDBG_MSG(("b_mp4_player_movie:%#lx " B_MP4_TYPE_FORMAT "[" B_MP4_TYPE_FORMAT "] %u bytes", (unsigned long)player, B_MP4_TYPE_ARG(handler->type), B_MP4_TYPE_ARG(type), box?(unsigned)batom_len(box):0));
	BDBG_OBJECT_ASSERT(player, bmp4_player_t);
	switch(type) {
	case BMP4_MOVIEHEADER:
		BDBG_ASSERT(box);
		player->movie.movieheader_valid = bmp4_parse_movieheader(box, &player->movie.movieheader);
		if(!player->movie.movieheader_valid) {
			BDBG_WRN(("b_mp4_player_movie: %#lx error in parsing movie header", (unsigned long)player));
		}
		break;
	case BMP4_TYPE_END:
		BDBG_MSG(("b_mp4_player_movie:%#lx parsing completed", (unsigned long) player));
		player->movie.valid = true;
		action = bmp4_parser_action_return;
		break;
	default:
		break;
	}
	if(box) {
		batom_release(box);
	}
	return action;
}

static bmp4_parser_action  
b_mp4_player_filetype(bmp4_parser_handler *handler, uint32_t type, batom_t box)
{
	bmp4_player_t player = ((b_mp4_player_handler *)handler)->player;

	BDBG_MSG(("b_mp4_player_filetype:%p " B_MP4_TYPE_FORMAT "[" B_MP4_TYPE_FORMAT "] %u bytes", (void *)player, B_MP4_TYPE_ARG(handler->type), B_MP4_TYPE_ARG(type), box?(unsigned)batom_len(box):0));
	BDBG_OBJECT_ASSERT(player, bmp4_player_t);
	if(type == BMP4_FILETYPEBOX) {
		BDBG_ASSERT(box);
		player->movie.filetype_valid = bmp4_parse_filetype(box, &player->movie.filetype);
	}
	if(box) {
		batom_release(box);
	}
	return bmp4_parser_action_none;
}


static void
b_mp4_track_init(b_mp4_track *track)
{
    track->trackheader_valid = false;
    track->mediaheader_valid = false;
    track->track_extends_valid = false;
    track->handler_valid = false;
    track->sample_valid = false;
    track->next_sample_valid = false;
    track->fragment_valid = false;
    track->attached = false;
    track->stream = NULL;
    track->hdr_len = 0;
    track->accum_data = NULL;
    track->track_accum = NULL;
    track->bounded_pes = false;
    track->fragment.index.valid = false;
    bmp4_track_info_init(&track->info);
    return;
}

static void
b_mp4_fragment_track_clear(b_mp4_fragment_track *track)
{
    track->next_sample_valid = false;
    track->fragment_run_valid = false;
    track->fragment_header_valid = false;
    track->movie_fragment_valid = false;
    track->movie_fragment_data_valid = false;
    track->state.current.run_offset = 0;
    track->state.current.run_start_time = 0;
    track->state.next.run_offset = 0;
    track->state.next.run_start_time = 0;
    return;
}

static void
b_mp4_fragment_track_reset(bmp4_player_t player, b_mp4_fragment_track *track)
{
    b_mp4_fragment_track_clear(track);
    track->state.trick.index_entry = 0;
    track->state.trick.end_of_index = 0;
    b_mp4_active_box_set(&track->root, player->first_movie_fragment.start, (uint64_t)(-1), 0);
    return;
}

static BERR_Code
b_mp4_fragment_track_init(bmp4_player_t player, b_mp4_fragment_track *track)
{
    BERR_Code rc;

    if(bfile_cached_segment_init(&track->cache, player->fragment.buffer, player->factory, BIO_BLOCK_SIZE)!=0) { rc=BERR_TRACE(BERR_UNKNOWN);goto err_cache;}
    b_mp4_active_box_init(&track->movie_fragment, BMP4_MOVIE_FRAGMENT);
    b_mp4_active_box_init(&track->movie_fragment_data, BMP4_MOVIE_DATA);
    b_mp4_active_box_init(&track->track_fragment, BMP4_TRACK_FRAGMENT);
    b_mp4_active_box_init(&track->track_fragment_run, BMP4_TRACK_FRAGMENT_RUN);
    b_mp4_active_box_init(&track->root, BMP4_TYPE('r','o','o','t'));
    b_mp4_fragment_track_reset(player, track);
    return BERR_SUCCESS;

err_cache:
    return rc;
}

static void
b_mp4_fragment_track_shutdown(b_mp4_fragment_track *track)
{
    bfile_cached_segment_shutdown(&track->cache);
    return;
}

static void
b_mp4_player_free_track(b_mp4_track *track)
{
    if(track->sample_valid) {
        bmp4_free_sample_info(&track->info.sample_info);
    }
    if(track->stream) {
        bmp4_stream_sample_destroy(track->stream);
    }
    if(track->fragment.index.valid) {
        bfile_cache_destroy(track->fragment.index.tfra_cache);
    }
    if(track->fragment_valid) {
        b_mp4_fragment_track_shutdown(&track->fragment);
    }
    batom_accum_destroy(track->track_accum);
    if(track->accum_data) {
        BKNI_Free(track->accum_data);
    }
    BKNI_Free(track);
    return;
}

static void
b_mp4_player_tracks_free(bmp4_player_t player)
{
	b_mp4_track *track;

	while(NULL!=(track = BLST_SQ_FIRST(&player->movie.tracks))) {
		BLST_SQ_REMOVE_HEAD(&player->movie.tracks, next);
        b_mp4_player_free_track(track);
	}
	return;
}

static b_mp4_track *
b_mp4_player_create_track(bmp4_player_t player)
{
    b_mp4_track *track;
    track = BKNI_Malloc(sizeof(*track));
    if(track) {
        b_mp4_track_init(track);
        track->track_accum = batom_accum_create(player->factory);
        if(track->track_accum==NULL) {
            (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
            BKNI_Free(track);
            track = NULL;
        }
    }
    return track;
}

static void
b_mp4_player_attach_track(bmp4_player_t player, b_mp4_track *track)
{
    BLST_SQ_INSERT_TAIL(&player->movie.tracks, track, next);
    track->attached = true;
    player->movie.ntracks++;
    return;
}

static b_mp4_track *
b_mp4_player_find_track(bmp4_player_t player, uint32_t track_ID)
{
    b_mp4_track *track;

    for(track = BLST_SQ_FIRST(&player->movie.tracks); track!=NULL ; track=BLST_SQ_NEXT(track, next)) {
        if ((track->trackheader_valid && track->info.trackheader.track_ID == track_ID) || (track->track_extends_valid && track->track_extends.track_ID==track_ID)) {
            return track;
        }
    }
	BDBG_MSG(("b_mp4_player_find_track:%#lx creating new track #%u track_ID:%u", (unsigned long)player, player->movie.ntracks, track_ID));
    track = b_mp4_player_create_track(player);
    if(track) {
        b_mp4_player_attach_track(player, track);
    }
    return track;
}

static b_mp4_track *
b_mp4_player_reserve_track(bmp4_player_t player)
{
    b_mp4_track *track;
	track = player->current_track;
    if(track==NULL) {
        track = b_mp4_player_create_track(player);
        player->current_track = track;
    }
    return track;
}


static bmp4_parser_action  
b_mp4_player_track(bmp4_parser_handler *handler, uint32_t type, batom_t box)
{
	bmp4_player_t player = ((b_mp4_player_handler *)handler)->player;
	b_mp4_track *track;
	bmp4_parser_action action = bmp4_parser_action_none;
    bmp4_trackheaderbox trackheader;

	BDBG_MSG(("b_mp4_player_track:%p " B_MP4_TYPE_FORMAT "[" B_MP4_TYPE_FORMAT "] %u bytes", (void *)player, B_MP4_TYPE_ARG(handler->type), B_MP4_TYPE_ARG(type), box?(unsigned)batom_len(box):0));
	BDBG_OBJECT_ASSERT(player, bmp4_player_t);
	switch(type) {
	case BMP4_TRACKHEADER:
		BDBG_ASSERT(box);
	    if(bmp4_parse_trackheader(box, &trackheader) && bmedia_player_stream_test(&player->stream, trackheader.track_ID)) {
            track = player->current_track;
            if(track) {
                b_mp4_player_attach_track(player, track);
            } else {
                track = b_mp4_player_find_track(player, trackheader.track_ID);
                if(track==NULL) {
                    player->movie.error = true;
                    action = bmp4_parser_action_return;
                    break;
                }
                player->current_track = track;
            }
			track->trackheader_valid = true;
            track->info.trackheader = trackheader;
        }
		break;
	case BMP4_TYPE_BEGIN:
        player->current_track = NULL;
		break;
	case BMP4_TYPE_END:
	    track = player->current_track;
        if(!track->attached) {
	        player->current_track = NULL;
            b_mp4_player_free_track(track);
        }
        break;
	default:
		break;
	}
	if(box) {
		batom_release(box);
	}
	return action;
}

static bmp4_parser_action  
b_mp4_player_media(bmp4_parser_handler *handler, uint32_t type, batom_t box)
{
	bmp4_player_t player = ((b_mp4_player_handler *)handler)->player;
	b_mp4_track *track;
	bmp4_parser_action action = bmp4_parser_action_none;

	BDBG_MSG(("b_mp4_player_media:%p " B_MP4_TYPE_FORMAT "[" B_MP4_TYPE_FORMAT "] %u bytes", (void *)player, B_MP4_TYPE_ARG(handler->type), B_MP4_TYPE_ARG(type), box?(unsigned)batom_len(box):0));
	BDBG_OBJECT_ASSERT(player, bmp4_player_t);
    track = b_mp4_player_reserve_track(player);
	if(track) {
		switch(type) {
		case BMP4_MEDIAHEADER:
			BDBG_ASSERT(box);
            track->mediaheader_valid = bmp4_parse_mediaheader(box, &track->info.mediaheader);
            if(!track->mediaheader_valid) {
                BDBG_WRN(("b_mp4_player_media: %#lx track:%u error in parsing Media Header", (unsigned long)player, track->info.trackheader.track_ID));
            }
			break;
		case BMP4_HANDLER:
			BDBG_ASSERT(box);
			track->handler_valid = bmp4_parse_handler(box, &track->info.handler);
			if(!track->handler_valid) {
				BDBG_WRN(("b_mp4_player_media: %#lx track:%u error in parsing Handler Reference", (unsigned long)player, track->info.trackheader.track_ID));
			}
			break;
		default:
			break;
		}
	}
	if(box) {
		batom_release(box);
	}
	return action;
}

static bmp4_parser_action  
b_mp4_player_mediainfo(bmp4_parser_handler *handler, uint32_t type, batom_t box)
{
	bmp4_player_t player = ((b_mp4_player_handler *)handler)->player;
	bmp4_parser_action action = bmp4_parser_action_none;

	BSTD_UNUSED(type);

	BDBG_MSG(("b_mp4_player_mediainfo:%p " B_MP4_TYPE_FORMAT "[" B_MP4_TYPE_FORMAT "] %u bytes", (void *)player, B_MP4_TYPE_ARG(handler->type), B_MP4_TYPE_ARG(type), box?(unsigned)batom_len(box):0));
	BDBG_OBJECT_ASSERT(player, bmp4_player_t);
	if(box) {
		batom_release(box);
	}
	return action;
}

static bmp4_parser_action  
b_mp4_player_sampletable(bmp4_parser_handler *handler, uint32_t type, batom_t box)
{
	bmp4_player_t player = ((b_mp4_player_handler *)handler)->player;
	b_mp4_track *track;
	bmp4_parser_action action = bmp4_parser_action_none;

	BDBG_MSG(("b_mp4_player_sampletable:%p " B_MP4_TYPE_FORMAT "[" B_MP4_TYPE_FORMAT "] %u bytes", (void *)player, B_MP4_TYPE_ARG(handler->type), B_MP4_TYPE_ARG(type), box?(unsigned)batom_len(box):0));
	BDBG_OBJECT_ASSERT(player, bmp4_player_t);
    track = b_mp4_player_reserve_track(player);
	if(track) {
		switch(type) {
		case BMP4_SAMPLEDESCRIPTION:
			if(track->handler_valid) {
				track->sample_valid = bmp4_parse_sample_info(box, &track->info.sample_info, track->info.handler.handler_type);
				if(!track->sample_valid) {
					BDBG_WRN(("b_mp4_player_media: %#lx track:%u error in parsing Sample Description", (unsigned long)player, track->info.trackheader.track_ID));
				}
			} else {
				BDBG_WRN(("b_mp4_player_media: %#lx track:%u Handler Reference invalid, can't parse Sample Description", (unsigned long)player, track->info.trackheader.track_ID));
			}
			break;
		default:
			break;
		}
	}
	if(box) {
		batom_release(box);
	}
	return action;
}

static bmp4_parser_action  
b_mp4_player_movie_extends(bmp4_parser_handler *handler, uint32_t type, batom_t box)
{
	bmp4_player_t player = ((b_mp4_player_handler *)handler)->player;
	bmp4_parser_action action = bmp4_parser_action_none;


	BDBG_MSG(("b_mp4_player_movie_extends:%p " B_MP4_TYPE_FORMAT "[" B_MP4_TYPE_FORMAT "] %u bytes", (void *)player, B_MP4_TYPE_ARG(handler->type), B_MP4_TYPE_ARG(type), box?(unsigned)batom_len(box):0));
	BDBG_OBJECT_ASSERT(player, bmp4_player_t);
    if(type==BMP4_TRACK_EXTENDS) {
        bmp4_trackextendsbox track_extends;
        BDBG_ASSERT(box);
        if(bmp4_parse_trackextends(box, &track_extends) &&  bmedia_player_stream_test(&player->stream, track_extends.track_ID)) {
            b_mp4_track *track = b_mp4_player_find_track(player, track_extends.track_ID);
            if(track) {
                track->track_extends_valid = true;
                track->track_extends = track_extends;
            }
        }
    }
	if(box) {
		batom_release(box);
	}
	return action;
}


static bmp4_parser_action
b_mp4_player_object_begin(void *cntx, uint32_t type, uint64_t size, uint64_t offset, size_t header_size)
{
    bmp4_player_t player = cntx;
    bfile_segment *segment=NULL;
    b_mp4_track *track;

    BDBG_MSG(("b_mp4_player_object_begin:%p " B_MP4_TYPE_FORMAT " %u bytes at %#lx", (void *)cntx, B_MP4_TYPE_ARG(type), (unsigned)size, (unsigned long)offset));
    BDBG_OBJECT_ASSERT(player, bmp4_player_t);
    if(type==BMP4_MOVIE_DATA) {
        player->next_seek = offset+size;
        return bmp4_parser_action_return;
    } else if (type==BMP4_MOVIE_FRAGMENT) {
        BDBG_ASSERT(offset>=header_size);
        bfile_segment_set(&player->first_movie_fragment, offset-header_size, size+header_size);
        player->fragmented = true;
        return bmp4_parser_action_none;
    } else if(type==BMP4_MOVIE_EXTENDS) {
        player->fragmented = true;
        return bmp4_parser_action_return;
    }

    switch(type) {
    case BMP4_DECODETIMETOSAMPLE:
        track = b_mp4_player_reserve_track(player);
        if(track) {
            segment = &track->info.decode_t2s;
        }
        break;
    case BMP4_COMPOSITIONTIMETOSAMPLE:
        track = b_mp4_player_reserve_track(player);
        if(track) {
            segment = &track->info.composition_t2s;
        }
        break;
    case BMP4_SAMPLESIZE:
        track = b_mp4_player_reserve_track(player);
        if(track) {
            segment = &track->info.samplesize;
        }
        break;
    case BMP4_COMPACTSAMPLESIZE:
        track = b_mp4_player_reserve_track(player);
        if(track) {
            segment = &track->info.usamplesize;
        }
        break;
    case BMP4_SAMPLETOCHINK:
        track = b_mp4_player_reserve_track(player);
        if(track) {
            segment = &track->info.sampletochunk;
        }
        break;
    case BMP4_CHUNKOFFSET:
        track = b_mp4_player_reserve_track(player);
        if(track) {
            segment = &track->info.chunkoffset;
        }
        break;
    case BMP4_CHUNKLARGEOFFSET:
        track = b_mp4_player_reserve_track(player);
        if(track) {
            segment = &track->info.chunkoffset64;
        }
        break;
    case BMP4_SYNCSAMPLE:
        track = b_mp4_player_reserve_track(player);
        if(track) {
            segment = &track->info.syncsample;
        }
        break;
    case BMP4_EDIT:
        track = b_mp4_player_reserve_track(player);
        if(track) {
            segment = &track->info.edit;
        }
        break;
    case BMP4_TYPE('s','t','s','h'): /* Shadow Sync Sample Box */
    case BMP4_TYPE('s','t','d','p'): /* Degradation Priority Box */
    case BMP4_TYPE('p','a','d','b'): /* Padding Bits Box */
    case BMP4_TYPE('s','d','t','p'): /* AVC, Independent and Disposable Samples Box */
    case BMP4_TYPE('s','b','g','p'): /* AVC, SampleToGroup Box */
        player->next_seek = offset+size;
        return bmp4_parser_action_discard;
    default:
        break;
    }
    if(segment==NULL) {
        return bmp4_parser_action_none;
    }
    BDBG_MSG(("b_mp4_player_object_begin:%#lx  segment:" B_MP4_TYPE_FORMAT " at %u:%u", (unsigned long)player, B_MP4_TYPE_ARG(type), (unsigned)offset, (unsigned) size));
    player->next_seek = offset+size;
    segment->start = offset;
    if(size<(1ul<<31)) {
        segment->len = size;
    } else {
        BDBG_WRN(("b_mp4_player_object_begin:%#lx truncationg length of " B_MP4_TYPE_FORMAT , (unsigned long)player, B_MP4_TYPE_ARG(type)));
        segment->len = 1ul<<31;
    }
    return bmp4_parser_action_discard;
}

static void
b_mp4_player_prepare_track(bmp4_player_t player, b_mp4_track *track)
{
    batom_accum_t dst = player->accum_dest;
    batom_cursor cursor;
    const bmp4_sampleentry *sample = track->info.sample_info.entries[0];

    switch(sample->sample_type) {
    case bmp4_sample_type_avc:
        {
            const bmp4_sample_avc *avc = &sample->codec.avc;
            bmedia_copy_h264_meta(dst, &avc->meta);
            batom_cursor_from_accum(&cursor, dst);
            track->hdr_len = batom_cursor_copy(&cursor, track->hdr_buf, sizeof(track->hdr_buf));
            if(track->hdr_len==sizeof(track->hdr_buf)) {
                BDBG_ERR(("b_mp4_player_make_header: %p AVC truncated header %u:%u", (void *)player, (unsigned)batom_accum_len(dst), (unsigned)track->hdr_len));
            }
            batom_accum_clear(dst);
        }
        break;
    case bmp4_sample_type_hevc:
        {
            const bmp4_sample_hevc *hevc = &sample->codec.hevc;
            bmedia_copy_h265_meta(dst, &hevc->meta, &bmedia_nal_vec);
            batom_cursor_from_accum(&cursor, dst);
            track->hdr_len = batom_cursor_copy(&cursor, track->hdr_buf, sizeof(track->hdr_buf));
            if(track->hdr_len==sizeof(track->hdr_buf)) {
                BDBG_ERR(("b_mp4_player_make_header: %p HEVC truncated header %u:%u", (void *)player, (unsigned)batom_accum_len(dst), (unsigned)track->hdr_len));
            }
            batom_accum_clear(dst);
        }
        break;
    case bmp4_sample_type_mp4a:
        {
            const bmp4_sample_mp4a *mp4a = &sample->codec.mp4a;
            bmedia_info_aac aac;

            aac.sampling_frequency_index = mp4a->mpeg4.decoder.iso_14496_3.samplingFrequencyIndex;
            aac.channel_configuration = mp4a->mpeg4.decoder.iso_14496_3.channelConfiguration;
            aac.profile = mp4a->mpeg4.decoder.iso_14496_3.audioObjectType;
            bmedia_adts_header_init_aac(&track->codec.mp4a, &aac);
        }
        break;
    case bmp4_sample_type_samr:
    case bmp4_sample_type_sawb:
    case bmp4_sample_type_sawp:
        {
            const bmp4_sample_amr *amr = &sample->codec.amr;
            uint8_t *buf = track->hdr_buf;
            bmedia_waveformatex_header waveformat;

            buf[0] = 'B'; buf[1]='C'; buf[2]='M'; buf[3]='A';
            buf[4] = 0;   buf[5]=0;   buf[6]=0;   buf[7]=0; /* place holder for the size */
            bmedia_init_waveformatex(&waveformat);
            waveformat.wFormatTag = sample->sample_type == bmp4_sample_type_samr ? 0x0057 : 0x0058; /* vlc_codecs.h */
            waveformat.nChannels = amr->audio.channelcount;
            waveformat.nSamplesPerSec = amr->audio.samplerate>>16;
            waveformat.nBlockAlign = 1;
            waveformat.wBitsPerSample = amr->audio.samplesize;
            waveformat.cbSize = 0;
            waveformat.nAvgBytesPerSec = waveformat.nSamplesPerSec * waveformat.nBlockAlign;
            track->hdr_len = bmedia_write_waveformatex(track->hdr_buf+8, &waveformat) + 8;
        }
        break;
    case bmp4_sample_type_als:
        {
            const bmp4_sample_mp4a *mp4a =  &sample->codec.mp4a;
            uint8_t *buf = track->hdr_buf;
            bmedia_waveformatex_header waveformat;

            buf[0] = 'B'; buf[1]='C'; buf[2]='M'; buf[3]='A';
            buf[4] = 0;   buf[5]=0;   buf[6]=0;   buf[7]=0; /* place holder for the size */
            bmedia_init_waveformatex(&waveformat);
            waveformat.wFormatTag = 0x00FF; /* RAW AAC */
            waveformat.nChannels = mp4a->audio.channelcount;
            waveformat.nSamplesPerSec = mp4a->audio.samplerate>>16;
            waveformat.nBlockAlign = 1;
            waveformat.wBitsPerSample = mp4a->audio.samplesize;
            waveformat.cbSize = mp4a->mpeg4.decoder.iso_14496_3.aac_info_size;
            track->hdr_len = bmedia_write_waveformatex(track->hdr_buf+8, &waveformat) + 8;
        }
        break;

    case bmp4_sample_type_qt_ima_adpcm:
        {
            const bmp4_sample_ms *ms = &sample->codec.ms;
            uint8_t *buf = track->hdr_buf;
            size_t meta_length;
            buf[0] = 'B'; buf[1]='C'; buf[2]='M'; buf[3]='A';
            buf[4] = 0;   buf[5]=0;   buf[6]=0;   buf[7]=0; /* place holder for the size */
            meta_length = ms->waveformat.meta_length;
            BDBG_CASSERT(sizeof(track->hdr_buf)-8 >= sizeof(ms->waveformat.meta));
            BKNI_Memcpy(buf+8, ms->waveformat.meta, meta_length);
            track->hdr_len = 8+meta_length;
            track->codec.pcm.pcm_packet_size = bmedia_waveformatex_pcm_block_size(&ms->waveformat);
        }
        break;
    case bmp4_sample_type_qt_ima4_adpcm:
        {
            const bmp4_sample_ima4 *ima4 = &sample->codec.ima4;
            uint8_t *buf = track->hdr_buf;
            size_t meta_length;
            bmedia_waveformatex_header waveformat;

            buf[0] = 'B'; buf[1]='C'; buf[2]='M'; buf[3]='A';
            buf[4] = 0;   buf[5]=0;   buf[6]=0;   buf[7]=0; /* place holder for the size */
            bmedia_init_waveformatex(&waveformat);
            waveformat.wFormatTag = BMEDIA_WAVFMTEX_AUDIO_BCMA_QT_IMA4_TAG;
            waveformat.nChannels = ima4->audio.channelcount;
            waveformat.nSamplesPerSec = ima4->audio.samplerate>>16;
            waveformat.nBlockAlign = 34 * ima4->audio.channelcount;
            waveformat.wBitsPerSample = ima4->audio.samplesize;
            waveformat.cbSize = 0;
            waveformat.nAvgBytesPerSec = waveformat.nSamplesPerSec * waveformat.nBlockAlign;

            meta_length = bmedia_write_waveformatex(track->hdr_buf+8, &waveformat);
            track->hdr_len = 8+meta_length;
            track->codec.pcm.pcm_packet_size = bmedia_waveformatex_pcm_block_size((bmedia_waveformatex *)&waveformat);
        }
        break;
    case bmp4_sample_type_twos:
        {
            const bmp4_sample_twos *twos= &sample->codec.twos;
            uint8_t *buf = track->hdr_buf;
            size_t meta_length;
            bmedia_waveformatex_header waveformat;

            buf[0] = 'B'; buf[1]='C'; buf[2]='M'; buf[3]='A';
            buf[4] = 0;   buf[5]=0;   buf[6]=0;   buf[7]=0; /* place holder for the size */
            bmedia_init_waveformatex(&waveformat);
            waveformat.wFormatTag = twos->type == BMP4_SAMPLE_TWOS ? BMEDIA_WAVFMTEX_AUDIO_PCM_BE_TAG : BMEDIA_WAVFMTEX_AUDIO_PCM_TAG;
            waveformat.nChannels = twos->audio.channelcount;
            waveformat.nSamplesPerSec = twos->audio.samplerate>>16;
            waveformat.nBlockAlign = twos->audio.channelcount * twos->audio.samplesize/8;
            waveformat.wBitsPerSample = twos->audio.samplesize;
            waveformat.cbSize = 0;
            waveformat.nAvgBytesPerSec = waveformat.nSamplesPerSec * waveformat.nBlockAlign;

            meta_length = bmedia_write_waveformatex(track->hdr_buf+8, &waveformat);
            track->hdr_len = 8+meta_length;
            track->codec.pcm.pcm_packet_size = bmedia_waveformatex_pcm_block_size((bmedia_waveformatex *)&waveformat);
        }
        break;
    case bmp4_sample_type_mp4v:
        {
            const bmp4_sample_mp4v *mp4v = &sample->codec.mp4v;
            track->hdr_len = mp4v->mpeg4.decoder.iso_14496_2.header_size;
            if(track->hdr_len>=sizeof(track->hdr_buf)) {
                track->hdr_len = sizeof(track->hdr_buf);
                BDBG_WRN(("b_mp4_player_make_header: %p MP4V truncated header %u:%u", (void *)player, (unsigned)batom_accum_len(dst), (unsigned)track->hdr_len));
            }
            if(track->hdr_len >= sizeof(mp4v->mpeg4.decoder.iso_14496_2.header)) {/* this wouldn't never happen because of the code in bmpeg4_parse_es_descriptor, however static analysis tools don't know that */
                track->hdr_len = sizeof(mp4v->mpeg4.decoder.iso_14496_2.header);
            }
            BKNI_Memcpy(track->hdr_buf, mp4v->mpeg4.decoder.iso_14496_2.header, track->hdr_len);
        }
        break;
    case bmp4_sample_type_mjpeg:
        track->bounded_pes = true;
        break;

    case bmp4_sample_type_dts:
    case bmp4_sample_type_ac3:
    case bmp4_sample_type_eac3:
    default:
        break;
    }
    return;
}

static ssize_t
b_mp4_player_feed_single_block(bmp4_player_t player, bmp4_parser_t mp4, batom_pipe_t pipe_mp4)
{
    void *buf;
    ssize_t read_result;
    size_t mp4_result=-1;
	batom_t atom;
	BERR_Code mrc;
	const size_t len = 4096;
	bfile_io_read_t fd = player->fd;
	batom_factory_t factory=player->factory;

    buf = BKNI_Malloc(len); 
    if (!buf) {
        goto err_alloc;
    }
    read_result =  fd->read(fd, buf, len);
    BDBG_MSG(("b_mp4_player_feed_single_block: %p read %d bytes", (void *)player, (int)read_result));
    if(read_result<0) {
        mrc = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto err_read;
    }
    if(read_result==0) {
        if(player->movie.valid) { /* stop parsing once reached the moov object */
            mp4_result = 0;
            goto err_read;
        }
        mp4_result = bmp4_parser_feed(mp4, pipe_mp4); /* otherwise run through accumulated data */
        if(mp4_result>0) {
            BKNI_Free(buf);
            return mp4_result;
        } else { /* return error if nothing was parsed */
            BDBG_WRN(("b_mp4_player_feed_single_block: %#lx reached end of file", (unsigned long)player));
            goto err_read;
        }
    }
    atom = batom_from_range(factory, buf, read_result, &b_mp4_player_atom, &buf);
    if(!atom) {
        mrc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_atom;
    }
    batom_pipe_push(pipe_mp4, atom);
    mp4_result = bmp4_parser_feed(mp4, pipe_mp4);
    return mp4_result;

err_atom:
err_read:
    BKNI_Free(buf);
err_alloc:
    return mp4_result;
}

static BERR_Code
b_mp4_player_feed_handle_seek(bmp4_player_t player, bmp4_parser_t mp4, batom_pipe_t pipe_mp4)
{
    BERR_Code rc;
    if(player->next_seek>0) {
        uint64_t next_seek = player->next_seek;
        bmp4_parser_status status;
        bfile_io_read_t fd = player->fd;

        player->next_seek= 0;
        bmp4_parser_get_status(mp4, &status);
        if(next_seek>(status.offset+status.acc_length)) { /* only skip forward to jump through junk data */
            off_t seek_result;

            batom_pipe_flush(pipe_mp4);
            BDBG_MSG(("b_mp4_player_open_file: seek to " B_OFFT_FMT " (skip " B_OFFT_FMT ")", B_OFFT_ARG(next_seek), B_OFFT_ARG(next_seek-(status.offset+status.acc_length)) ));
            seek_result = fd->seek(fd, next_seek, SEEK_SET);
            if(seek_result != (off_t)next_seek) {
                rc = BERR_TRACE(BERR_NOT_SUPPORTED);
                goto err_seek;
            }
            bmp4_parser_seek(mp4, next_seek); /* skip mdat object */
        }
    }
    return BERR_SUCCESS;
err_seek:
    return BERR_TRACE(rc);
}


static BERR_Code
b_mp4_player_fragment_init(bmp4_player_t player)
{
    BERR_Code rc;
    size_t buffer_len = 8*BIO_BLOCK_SIZE;
    bfile_buffer_cfg buffer_cfg;
    b_mp4_track *track;

    if(player->config.format.mp4.fragmentBufferSize) {
        buffer_len = player->config.format.mp4.fragmentBufferSize;
    }

    player->fragment.buffer_data = BKNI_Malloc(buffer_len);
    if(!player->fragment.buffer_data) { rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_buffer_data;}
	bfile_buffer_default_cfg(&buffer_cfg);
	buffer_cfg.buf = player->fragment.buffer_data;
	buffer_cfg.buf_len = buffer_len;
	buffer_cfg.nsegs = buffer_cfg.buf_len / BIO_BLOCK_SIZE;
	buffer_cfg.fd = player->fd;
	player->fragment.buffer = bfile_buffer_create(player->factory, &buffer_cfg);
    if(!player->fragment.buffer) { rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_buffer;}
    for(track = BLST_SQ_FIRST(&player->movie.tracks); track!=NULL ; track=BLST_SQ_NEXT(track, next)) {
		if(track->trackheader_valid && track->mediaheader_valid && track->handler_valid && track->sample_valid && track->track_extends_valid) {
            rc = b_mp4_fragment_track_init(player, &track->fragment);
            if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto err_track;}
            track->fragment_valid = true;
        }
    }
    return BERR_SUCCESS;

err_track:
err_buffer:
    BKNI_Free(player->fragment.buffer_data);
err_buffer_data:
    return rc;
}

static void
b_mp4_player_fragment_shutdown(bmp4_player_t player)
{
    if(player->fragment.buffer) {
        bfile_buffer_destroy(player->fragment.buffer);
    }
    if(player->fragment.buffer_data) {
        BKNI_Free(player->fragment.buffer_data);
    }
    return;
}

struct b_mp4_player_find_box_state {
    uint8_t box_payload[BMP4_BOX_MAX_SIZE + BMP4_FULLBOX_SIZE + 4];
    batom_vec vec;
    batom_cursor cursor;
    bmp4_box box;
};

static size_t
b_mp4_player_find_box(bmp4_player_t player, off_t box_offset, uint32_t type, struct b_mp4_player_find_box_state *state)
{
    int rc;
    size_t box_header;
    size_t box_size;
    bfile_io_read_t fd;

    fd = player->fd;
    BDBG_MSG(("b_mp4_player_find_box:%p looking " B_MP4_TYPE_FORMAT " at %u", (void *)player, B_MP4_TYPE_ARG(type), (unsigned)box_offset));
    if(box_offset != fd->seek(fd, box_offset, SEEK_SET)) {
        BDBG_MSG(("b_mp4_player_find_box:%p can't seek to %u", (void *)player, (unsigned)box_offset));
        goto error;
    }
    rc = fd->read(fd, state->box_payload, sizeof(state->box_payload));
    if(rc!=sizeof(state->box_payload)) {
        BDBG_MSG(("b_mp4_player_find_box:%p can't read box(%d,%u)", (void *)player, rc, (unsigned)sizeof(state->box_payload)));
        goto error;
    }
    BATOM_VEC_INIT(&state->vec, state->box_payload, sizeof(state->box_payload));
    batom_cursor_from_vec(&state->cursor, &state->vec, 1);
    box_header = bmp4_scan_box(&state->cursor, type, &state->box);
    if(box_header == 0) {
        goto error;
    }
    box_size = state->box.size - box_header;
    return box_size;
error:
    return 0;
}


typedef struct bmp4_track_fragment_random_access_payload {
    uint64_t time;
    uint64_t moof_offset;
    unsigned traf_number;
    unsigned trun_number;
    unsigned sample_number;
} bmp4_track_fragment_random_access_payload;

#define BMP4_TRACK_FRAGMENT_RANDOM_ACCESS_HEADER_SIZE (BMP4_FULLBOX_SIZE + 4+4+4)

static bool
b_mp4_player_parse_track_fragment_random_access_header(batom_cursor *cursor, bmp4_track_fragment_random_access_header *tfra)
{
    uint32_t data;

    tfra->track_ID = batom_cursor_uint32_be(cursor);
    data = batom_cursor_uint32_be(cursor);
    tfra->length_size_of_traf_num = B_GET_BITS(data, 5, 4);
    tfra->length_size_of_trun_num = B_GET_BITS(data, 3, 2);
    tfra->length_size_of_sample_num = B_GET_BITS(data, 1, 0);
    tfra->number_of_entry = batom_cursor_uint32_be(cursor);
    BDBG_MSG(("b_mp4_player_parse_track_fragment_random_access_header: track_ID:%u length_size_of_traf_num:%u length_size_of_traf_num:%u length_size_of_sample_num:%u number_of_entry:%u", (unsigned)tfra->track_ID, tfra->length_size_of_traf_num, tfra->length_size_of_trun_num, tfra->length_size_of_sample_num, (unsigned)tfra->number_of_entry));
    return !BATOM_IS_EOF(cursor);
}

static bool
b_mp4_player_read_track_fragment_random_access_payload(b_mp4_track *track, unsigned entry, bmp4_track_fragment_random_access_payload *payload)
{
    int rc;
    const uint8_t *tfra_data;
    batom_vec vec;
    batom_cursor cursor;

    rc = bfile_cache_seek(track->fragment.index.tfra_cache, entry * track->fragment.index.tfra_entry_size);
    if(rc!=0) {
        BDBG_WRN(("b_mp4_player_read_track_fragment_random_access_payload:%p(%u) can't access entry %u", (void *)track, track->info.trackheader.track_ID, entry));
        return false;
    }
    tfra_data = bfile_cache_next(track->fragment.index.tfra_cache);
    if(tfra_data==NULL) {
        BDBG_WRN(("b_mp4_player_read_track_fragment_random_access_payload:%p(%u) can't read entry %u", (void *)track, track->info.trackheader.track_ID, entry));
        return false;
    }
    BATOM_VEC_INIT(&vec, tfra_data, track->fragment.index.tfra_entry_size);
    batom_cursor_from_vec(&cursor, &vec, 1);
    if(track->fragment.index.tfra_version==1) {
        payload->time = batom_cursor_uint64_be(&cursor);
        payload->moof_offset = batom_cursor_uint64_be(&cursor);
    } else {
        payload->time = batom_cursor_uint32_be(&cursor);
        payload->moof_offset = batom_cursor_uint32_be(&cursor);
    }
    payload->traf_number = batom_cursor_vword_be(&cursor, track->fragment.index.tfra_header.length_size_of_traf_num+1);
    payload->trun_number = batom_cursor_vword_be(&cursor, track->fragment.index.tfra_header.length_size_of_trun_num+1);
    payload->sample_number = batom_cursor_vword_be(&cursor, track->fragment.index.tfra_header.length_size_of_sample_num+1);
    BDBG_MSG(("b_mp4_player_read_track_fragment_random_access_payload:%p(%u) time:%u moof_offset:%u traf_number:%u trun_number:%u sample_number:%u", (void *)track, track->info.trackheader.track_ID, (unsigned)payload->time, (unsigned)payload->moof_offset, (unsigned)payload->traf_number, (unsigned)payload->trun_number, (unsigned)payload->sample_number));
    return !BATOM_IS_EOF(&cursor);
}

static size_t
b_mp4_player_tfra_entry_size( const bmp4_track_fragment_random_access_header *header, unsigned version)
{
    size_t size = 0;
    size += version == 1 ? BMEDIA_FIELD_LEN("time", uint64_t) + BMEDIA_FIELD_LEN("moof_offset", uint64_t)  : BMEDIA_FIELD_LEN("time", uint32_t) + BMEDIA_FIELD_LEN("moof_offset", uint32_t);
    size += header->length_size_of_traf_num + 1;
    size += header->length_size_of_trun_num + 1;
    size += header->length_size_of_sample_num + 1;
    return size;
}

static int
b_mp4_player_open_fragment_index(bmp4_player_t player)
{
    bfile_io_read_t fd;
    struct b_mp4_player_find_box_state box_state;
    off_t first, last; /* offset of the file */
    int rc;
    size_t mfro_payload;
    size_t mfra_payload;
    size_t mfra_payload_offset;
    uint32_t mfra_size;
    bmp4_fullbox fullbox;

    BDBG_MSG(("b_mp4_player_open_fragment_index:%p", (void *)player));
    fd = player->fd;
    rc = fd->bounds(fd, &first, &last);
    if(rc!=0) {
        BDBG_MSG(("b_mp4_player_open_fragment_index:%p file size is not available", (void *)player));
        rc = -1; goto done;
    }
    /*
     * ISO/IEC 14496-12
     * 8.39 Movie Fragment Random Access Offset Box
     */
    if(last < (int)sizeof(box_state.box_payload)) {
        BDBG_MSG(("b_mp4_player_open_fragment_index:%p file too small %u", (void *)player, (unsigned)last));
        rc = -1; goto done;
    }
    mfro_payload = b_mp4_player_find_box(player, last - sizeof(box_state.box_payload), BMP4_TYPE('m','f','r','o'), &box_state);
    if(mfro_payload < 4 + BMP4_FULLBOX_SIZE) {
        BDBG_WRN(("b_mp4_player_open_fragment_index:%p invalid payload size %u", (void *)player, (unsigned)mfro_payload));
        rc = -1; goto done;
    }
    if(!bmp4_parse_fullbox(&box_state.cursor, &fullbox)) {
        rc = -1; goto done;
    }
    mfra_size = batom_cursor_uint32_be(&box_state.cursor);
    if(BATOM_IS_EOF(&box_state.cursor)) {
        BDBG_WRN(("b_mp4_player_open_fragment_index:%p not enough data", (void *)player));
        rc = -1; goto done;
    }
    if(last < (off_t)mfra_size) {
        BDBG_WRN(("b_mp4_player_open_fragment_index:%p invalid mfra_size(%u,%u)", (void *)player, (unsigned)last, (unsigned)mfra_size));
        rc = -1; goto done;
    }
    /*
     * ISO/IEC 14496-12
     * 8.38 Track Fragment Random Access Box
     */
    mfra_payload = b_mp4_player_find_box(player, last - mfra_size, BMP4_TYPE('m','f','r','a'), &box_state);
    for(mfra_payload_offset=batom_cursor_pos(&box_state.cursor); mfra_payload_offset < mfra_payload; ) {
        off_t box_offset = (last - mfra_size) + mfra_payload_offset;
        size_t tfra_payload = b_mp4_player_find_box(player, box_offset, BMP4_TYPE('t','f','r','a'), &box_state);
        bmp4_track_fragment_random_access_header tfra_header;
        b_mp4_track *track;
        bfile_segment tfra_header_payload;

        BDBG_CASSERT(BMP4_TRACK_FRAGMENT_RANDOM_ACCESS_HEADER_SIZE<sizeof(box_state.box_payload));
        if(tfra_payload < BMP4_FULLBOX_SIZE + BMP4_TRACK_FRAGMENT_RANDOM_ACCESS_HEADER_SIZE) {
            break;
        }
        mfra_payload_offset += box_state.box.size;
        box_offset += batom_cursor_pos(&box_state.cursor);
        if(box_offset != fd->seek(fd, box_offset, SEEK_SET)) {
            BDBG_WRN(("b_mp4_player_open_fragment_index:%p can't seek to %u", (void *)player, (unsigned)box_offset));
            break;
        }
        rc = fd->read(fd, box_state.box_payload, BMP4_TRACK_FRAGMENT_RANDOM_ACCESS_HEADER_SIZE);
        if(BMP4_TRACK_FRAGMENT_RANDOM_ACCESS_HEADER_SIZE != rc) {
            BDBG_WRN(("b_mp4_player_find_box:%p can't read box(%d,%u)", (void *)player, rc, BMP4_TRACK_FRAGMENT_RANDOM_ACCESS_HEADER_SIZE));
            break;
        }
        BATOM_VEC_INIT(&box_state.vec, box_state.box_payload,BMP4_TRACK_FRAGMENT_RANDOM_ACCESS_HEADER_SIZE);
        batom_cursor_from_vec(&box_state.cursor, &box_state.vec, 1);
        if(!bmp4_parse_fullbox(&box_state.cursor, &fullbox)) {
            continue;
        }
        if(!b_mp4_player_parse_track_fragment_random_access_header(&box_state.cursor, &tfra_header)) {
            continue;
        }
        track = b_mp4_player_find_track(player, tfra_header.track_ID);
        if(!track) {
            BDBG_WRN(("b_mp4_player_find_box:%p unknown track:%u", (void *)player, tfra_header.track_ID));
            continue;
        }
        track->fragment.index.tfra_header = tfra_header;
        track->fragment.index.tfra_version = fullbox.version;
        box_offset += batom_cursor_pos(&box_state.cursor);
        track->fragment.index.tfra_entry_size = b_mp4_player_tfra_entry_size(&track->fragment.index.tfra_header, track->fragment.index.tfra_version);
        bfile_segment_set(&tfra_header_payload, box_offset, track->fragment.index.tfra_entry_size * track->fragment.index.tfra_header.number_of_entry);
        track->fragment.index.tfra_cache = bfile_cache_create(fd, &tfra_header_payload, 1024, track->fragment.index.tfra_entry_size);
        if(!track->fragment.index.tfra_cache) {
            BDBG_ERR(("b_mp4_player_open_fragment_index:%p can't create cache for tfra:%u", (void *)player, (unsigned)track->fragment.index.tfra_header.track_ID));
            break;
        }
        track->fragment.index.valid = true;
        player->fragment.index.valid = true;
    }

#if 0
    {
        b_mp4_track *track;
    for(track = BLST_SQ_FIRST(&player->movie.tracks); track ; track=BLST_SQ_NEXT(track, next)) {
        unsigned i;
        if(!track->fragment.index.valid) {
            continue;
        }
        for(i=0;i<track->fragment.index.tfra_header.number_of_entry;i++) {
            bmp4_track_fragment_random_access_payload payload;
            b_mp4_player_read_track_fragment_random_access_payload(track, i, &payload);
        }
    }
    }
#endif
    rc = 0;

done:
    return rc ;
}

static int
b_mp4_player_open_file(bmp4_player_t player)
{
    batom_pipe_t pipe_mp4;
    bmp4_parser_cfg cfg;
    bmp4_parser_t  mp4;
    enum {b_mp4_player_open_state_movi, b_mp4_player_open_state_idx} state;
    int rc=-1;
    batom_factory_t factory=player->factory;
    BERR_Code mrc;
    unsigned sample_count=0;
    bfile_io_read_t fd;
    ssize_t mp4_result;

    BSTD_UNUSED(mrc);
    BDBG_OBJECT_ASSERT(player, bmp4_player_t);
    state = b_mp4_player_open_state_movi;
    BLST_SQ_INIT(&player->movie.tracks);
    player->eof_reached = false;
    player->movie.ntracks = 0;
    player->movie.error = false;
    player->movie.valid = false;
    player->movie.movieheader_valid = false;
    player->movie.filetype_valid = false;
    player->next_seek = 0;
    player->filetype_handler.player = player;
    player->movie_handler.player = player;
    player->track_handler.player = player;
    player->media_handler.player = player;
    player->movie_extends_handler.player = player;
    player->mediainfo_handler.player = player;
    player->sampletable_handler.player = player;
    player->current_track = NULL;
    player->continuation_track = NULL;
    player->fragmented = false;
    player->fragment.index.valid = false;

    bmp4_parser_default_cfg(&cfg);
    cfg.user_cntx = player;
    cfg.box_begin = b_mp4_player_object_begin;
    mp4 = bmp4_parser_create(factory, &cfg);
    if(!mp4) {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_mp4;
    }
    bmp4_parser_install_handler(mp4, &player->filetype_handler.handler, BMP4_FILETYPEBOX, b_mp4_player_filetype);
    bmp4_parser_install_handler(mp4, &player->movie_handler.handler, BMP4_MOVIE, b_mp4_player_movie);
    bmp4_parser_install_handler(mp4, &player->track_handler.handler, BMP4_TRACK, b_mp4_player_track);
    bmp4_parser_install_handler(mp4, &player->media_handler.handler, BMP4_MEDIA, b_mp4_player_media);
    bmp4_parser_install_handler(mp4, &player->movie_extends_handler.handler, BMP4_MOVIE_EXTENDS, b_mp4_player_movie_extends);
    bmp4_parser_install_handler(mp4, &player->mediainfo_handler.handler, BMP4_MEDIAINFORMATION, b_mp4_player_mediainfo);
    bmp4_parser_install_handler(mp4, &player->sampletable_handler.handler, BMP4_SAMPLETABLE, b_mp4_player_sampletable);
    pipe_mp4 = batom_pipe_create(factory);
    if (!pipe_mp4) {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_pipe;
    }
    fd = player->fd;
    BDBG_MSG(("b_mp4_player_open_file: seek to 0"));
    fd->seek(fd, 0, SEEK_SET);
    while(!player->movie.valid) {

        mp4_result = b_mp4_player_feed_single_block(player, mp4, pipe_mp4);
        if(player->movie.valid) {
            break;
        }
        if(mp4_result==0) {
            BDBG_WRN(("b_mp4_player_open_file: %#lx unexpected stall in parsing", (unsigned long) player));
            goto err_parser;
        } else if(mp4_result<0) {
            goto err_parser;
        }
        if(player->movie.error) {
            BDBG_WRN(("b_mp4_player_open_file: %#lx error in the movie parsing", (unsigned long) player));
            goto err_parser;
        }
        mrc = b_mp4_player_feed_handle_seek(player, mp4, pipe_mp4);
        if(mrc!=BERR_SUCCESS) {
            rc = BERR_TRACE(mrc);
            goto err_seek;
        }
    }
    if(player->movie.movieheader_valid) {
        b_mp4_track *track;

        if(player->fragmented) { /* find a first fragment */
            mrc = b_mp4_player_feed_handle_seek(player, mp4, pipe_mp4);
            if(mrc!=BERR_SUCCESS) {
                rc = BERR_TRACE(mrc);
                goto err_seek;
            }
            for(;;) {
                mp4_result = b_mp4_player_feed_single_block(player, mp4, pipe_mp4);
                if(mp4_result==0) {
                    BDBG_WRN(("b_mp4_player_open_file: %#lx fragmented stream without fragments ", (unsigned long)player));
                    player->fragmented = false;
                    break;
                }
                if(bfile_segment_test(&player->first_movie_fragment)) {
                    BDBG_MSG(("b_mp4_player_open_file: %#lx fragmented stream, first fragment at %#lx", (unsigned long)player, (unsigned long)player->first_movie_fragment.start));
                    mrc = b_mp4_player_fragment_init(player);
                    if(mrc!=BERR_SUCCESS) { rc=-1;goto err_fragment; }
                    break;
                }
                mrc = b_mp4_player_feed_handle_seek(player, mp4, pipe_mp4);
                if(mrc!=BERR_SUCCESS) {
                    rc = BERR_TRACE(mrc);
                    goto err_seek;
                }
            }
        }

        for(track = BLST_SQ_FIRST(&player->movie.tracks); track ; track=BLST_SQ_NEXT(track, next)) {
            BDBG_MSG(("b_mp4_player_open_file: %#lx track:%u trackheader:%c mediaheader:%c handler:%c sample:%c", (unsigned long)player, track->info.trackheader.track_ID, track->trackheader_valid?'+':'-', track->mediaheader_valid?'+':'-', track->handler_valid?'+':'-', track->sample_valid?'+':'-'));
            if(track->trackheader_valid && track->mediaheader_valid && track->handler_valid && track->sample_valid) {
                bmp4_stream_sample_status status;

                track->pes_id = bmedia_player_stream_get_id(&player->stream, track->info.trackheader.track_ID);
                if(track->info.sample_info.entry_count!=1) {
                    BDBG_WRN(("b_mp4_player_open_file: %#lx unsupported number of entries (%u) in track %u",  (unsigned long)player, (unsigned)track->info.sample_info.entry_count, (unsigned)track->info.trackheader.track_ID));
                    continue;
                }
                BDBG_ASSERT(track->info.sample_info.entries[0]);
                if(track->info.sample_info.entries[0]->sample_type==bmp4_sample_type_unknown) {
                    BDBG_WRN(("b_mp4_player_open_file: %#lx unsupported codec " B_MP4_TYPE_FORMAT " in track %u->%#x", (unsigned long)player, B_MP4_TYPE_ARG(track->info.sample_info.entries[0]->type), track->info.trackheader.track_ID, track->pes_id));
                    /* continue */;
                }
                b_mp4_player_prepare_track(player, track);
                track->info.movieheader = &player->movie.movieheader;
                rc = 0;
                track->stream = bmp4_stream_sample_create(fd, &track->info);
                if(!track->stream) {
                    continue;
                }
                bmp4_stream_sample_get_status(track->stream, &status);
                BDBG_MSG(("b_mp4_player_open_file: %#lx created stream for track %u(%#lx) samples_count:%u", (unsigned long)player, track->info.trackheader.track_ID, (unsigned long)track, status.sample_count));
                sample_count += status.sample_count;
            }
        }
    }
    if(rc!=0) {
        BDBG_WRN(("b_mp4_player_open_file: %#lx can't find any mapped tracks", (unsigned long)player));
    }
    if(player->fragmented) {
        b_mp4_player_open_fragment_index(player);
    }

err_fragment:
err_seek:
err_parser:
    batom_pipe_flush(pipe_mp4);
    batom_pipe_destroy(pipe_mp4);
err_pipe:
    bmp4_parser_remove_handler(mp4, &player->sampletable_handler.handler);
    bmp4_parser_remove_handler(mp4, &player->mediainfo_handler.handler);
    bmp4_parser_remove_handler(mp4, &player->movie_extends_handler.handler);
    bmp4_parser_remove_handler(mp4, &player->media_handler.handler);
    bmp4_parser_remove_handler(mp4, &player->track_handler.handler);
    bmp4_parser_remove_handler(mp4, &player->movie_handler.handler);
    bmp4_parser_remove_handler(mp4, &player->filetype_handler.handler);
    bmp4_parser_destroy(mp4);
err_mp4:
    player->current_track = NULL;
    return rc;
}


bmp4_player_t
bmp4_player_create(bfile_io_read_t fd, const bmedia_player_config *config, const bmedia_player_stream *stream)
{
	bmp4_player_t player;
    BERR_Code rc;

    BSTD_UNUSED(rc);

	BDBG_ASSERT(fd);
	BDBG_ASSERT(config);
	BDBG_ASSERT(stream && stream->format == bstream_mpeg_type_mp4);

	if(!config->buffer) {
		BDBG_ERR(("bmp4_player_create: buffer has to be provided"));
		goto err_buffer;
	}
	player = BKNI_Malloc(sizeof(*player));
	if (!player) {
		BDBG_ERR(("bmp4_player_create: can't allocate %u bytes", (unsigned)sizeof(*player)));
		goto err_alloc;
	}
	BDBG_OBJECT_INIT(player, bmp4_player_t);
	player->fd = fd;
	player->config = *config;
	player->stream = *stream;
	player->status.direction = 0;
	player->status.index_error_cnt = 0;
	player->status.data_error_cnt = 0;
	player->status.bounds.first = 0;
	player->status.bounds.last = 0;
	player->status.position = 0;
	player->time_scale = BMEDIA_TIME_SCALE_BASE;
	player->rew_stack.position = 0;
	player->rew_stack.stack_next = 0;
    player->current_track = NULL;
    player->fragmented = false;
    player->fragment.buffer = NULL;
    player->fragment.buffer_data = NULL;

    bfile_segment_clear(&player->first_movie_fragment);

	player->factory = batom_factory_create(bkni_alloc, 64);
	if(!player->factory) {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		goto err_factory;
	}
	player->accum_src = batom_accum_create(player->factory);
	if(!player->accum_src) {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		goto err_accum_src;
	}
	player->accum_dest = batom_accum_create(player->factory);
	if(!player->accum_dest) {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		goto err_accum_dest;
	}

	if(b_mp4_player_open_file(player)<0) {
        rc = BERR_TRACE(BERR_NOT_SUPPORTED);
		goto err_index;
	}
	player->status.bounds.first = 0;
	player->status.bounds.last = (bmedia_player_pos)((1000*player->movie.movieheader.duration)/player->movie.movieheader.timescale);

	return player;
err_index:
	b_mp4_player_tracks_free(player);
	batom_accum_destroy(player->accum_dest);
err_accum_dest:
	batom_accum_destroy(player->accum_src);
err_accum_src:
	batom_factory_destroy(player->factory);
err_factory:
	BKNI_Free(player);
err_alloc:
err_buffer:
	return NULL;
}

void 
bmp4_player_destroy(bmp4_player_t player)
{
	BDBG_OBJECT_ASSERT(player, bmp4_player_t);
	b_mp4_player_tracks_free(player);
    if(player->fragmented) {
        b_mp4_player_fragment_shutdown(player);
    }
	batom_accum_destroy(player->accum_dest);
	batom_accum_destroy(player->accum_src);
	batom_factory_destroy(player->factory);
	BDBG_OBJECT_DESTROY(player, bmp4_player_t);
	BKNI_Free(player);

	return;
}


static void 
b_mp4_data_atom_free(batom_t atom, void *user)
{
	BDBG_MSG_TRACE(("b_mp4_data_atom_free:%#lx", (unsigned long)atom));
	BSTD_UNUSED(atom);
	BSTD_UNUSED(user);
	return;
}

static const batom_user b_mp4_data_atom = {
	b_mp4_data_atom_free,
	sizeof(void **)
};

static void
b_mp4_player_data_error(bmp4_player_t player)
{
    player->status.data_error_cnt++;
    BDBG_MSG_TRACE(("b_mp4_player_data_error:%p data_error:%u", (void *)player, (unsigned)player->status.data_error_cnt));
    player->config.error_detected(player->config.cntx);
    return;
}


static batom_t
b_mp4_process_sample_mp4a(bmp4_player_t player, b_mp4_track *track, batom_t atom, const bmp4_sample_mp4a *mp4a)
{
	batom_accum_t dst = player->accum_dest;
	size_t len = batom_len(atom)+7;

	BSTD_UNUSED(mp4a);
	BDBG_ASSERT(track);
	BDBG_ASSERT(mp4a);
	BDBG_ASSERT(batom_accum_len(dst)==0);

    len = bmedia_adts_header_fill(track->hdr_buf, &track->codec.mp4a, batom_len(atom));

	batom_accum_add_range(dst, track->hdr_buf, len);
	batom_accum_add_atom(dst, atom);
	batom_release(atom);
	return batom_from_accum(dst, NULL, NULL);
}

static batom_t
b_mp4_process_sample_amr(bmp4_player_t player, b_mp4_track *track, batom_t atom, const bmp4_sample_amr *amr)
{
	batom_accum_t dst = player->accum_dest;
	size_t len = batom_len(atom);
    uint8_t *buf = track->hdr_buf;

	BSTD_UNUSED(amr);
	BDBG_ASSERT(batom_accum_len(dst)==0);

    buf[4] = B_GET_BITS(len, 31, 24);
    buf[5] = B_GET_BITS(len, 23, 16);
    buf[6] = B_GET_BITS(len, 15, 8);
    buf[7] = B_GET_BITS(len, 7, 0);

	batom_accum_add_range(dst, track->hdr_buf, track->hdr_len);
	batom_accum_add_atom(dst, atom);
	batom_release(atom);
	return batom_from_accum(dst, NULL, NULL);
}

static batom_t
b_mp4_process_sample_als(bmp4_player_t player, b_mp4_track *track, batom_t atom, const bmp4_sample_mp4a *mp4a)
{
    batom_accum_t dst = player->accum_dest;
    size_t len = batom_len(atom);
    uint8_t *buf = track->hdr_buf;

    BDBG_ASSERT(batom_accum_len(dst)==0);

    buf[4] = B_GET_BITS(len, 31, 24);
    buf[5] = B_GET_BITS(len, 23, 16);
    buf[6] = B_GET_BITS(len, 15, 8);
    buf[7] = B_GET_BITS(len, 7, 0);

    batom_accum_add_range(dst, track->hdr_buf, track->hdr_len);
    batom_accum_add_range(dst, mp4a->mpeg4.decoder.iso_14496_3.aac_info, mp4a->mpeg4.decoder.iso_14496_3.aac_info_size);
    batom_accum_add_atom(dst, atom);
    batom_release(atom);
    return batom_from_accum(dst, NULL, NULL);
}

static batom_t
b_mp4_continue_sample_pcm(bmp4_player_t player, b_mp4_track *track)
{
    batom_cursor start;
    batom_cursor end;

    BSTD_UNUSED(player);

    batom_cursor_from_accum(&start, track->track_accum);
    BATOM_CLONE(&end, &start);
    BDBG_MSG(("b_mp4_continue_sample_pcm:%p data:%u packet_size:%u", (void *)track, (unsigned)batom_accum_len(track->track_accum), (unsigned)track->codec.pcm.pcm_packet_size));
    if(batom_cursor_skip(&end, track->codec.pcm.pcm_packet_size)==track->codec.pcm.pcm_packet_size) {
        batom_t atom;
        atom = batom_accum_extract(track->track_accum, &start, &end, NULL, NULL);
        if(atom) {
            uint8_t *buf = track->hdr_buf;
            size_t len = batom_len(atom);
            batom_vec hdr;
            batom_t frame;
            buf[4] = B_GET_BITS(len, 31, 24);
            buf[5] = B_GET_BITS(len, 23, 16);
            buf[6] = B_GET_BITS(len, 15, 8);
            buf[7] = B_GET_BITS(len, 7, 0);
            BATOM_VEC_INIT(&hdr, buf, track->hdr_len);
            batom_accum_trim(track->track_accum, &end);
            frame = batom_from_vec_and_atom(&hdr, atom, NULL, NULL);
            batom_release(atom);
            if(frame) {
                player->continuation_track = track;
            }
            return frame;
        }
    }
    return NULL;
}

static batom_t
b_mp4_process_sample_pcm(bmp4_player_t player, b_mp4_track *track, batom_t atom)
{
    const batom_vec *vecs;
    unsigned nvecs;
    batom_accum_t acc;

    BDBG_MSG(("b_mp4_process_sample_pcm:%#lx:%#lx", (unsigned long)player, (unsigned long)track));

    acc = track->track_accum;
    batom_accum_add_atom(acc, atom);
    batom_release(atom);
    vecs = batom_accum_get_vectors(acc, &nvecs);
    if(vecs && nvecs>16) { /* concatenate data from multiple samples */
        size_t sample_len = batom_accum_len(acc);
        void *accum_data = BKNI_Malloc(sample_len);
        if(accum_data) {
            batom_cursor cursor;
            batom_cursor_from_accum(&cursor, acc);
            batom_cursor_copy(&cursor, accum_data, sample_len);
            BDBG_MSG(("b_mp4_process_sample_pcm:%p:%p merging sample %u:%u", (void *)player, (void *)track, nvecs, (unsigned)sample_len));
            if(track->accum_data) {
                BKNI_Free(track->accum_data);
            }
            batom_accum_clear(acc);
            batom_accum_add_range(acc, accum_data, sample_len);
            track->accum_data = accum_data;
        }
    }
    return b_mp4_continue_sample_pcm(player, track);
}

static batom_t
b_mp4_process_sample_mp4v(bmp4_player_t player, b_mp4_track *track, batom_t atom, const bmp4_sample_mp4v *mp4v)
{
	batom_accum_t dst = player->accum_dest;

	BSTD_UNUSED(mp4v);
	BDBG_ASSERT(track);
	BDBG_ASSERT(mp4v);
	BDBG_ASSERT(batom_accum_len(dst)==0);

	if(track->next_sample.syncsample) {
		batom_accum_add_range(dst, track->hdr_buf, track->hdr_len);
		batom_accum_add_atom(dst, atom);
		batom_release(atom);
		return batom_from_accum(dst, NULL, NULL);
	} else {
		return atom;
	}
}

static batom_t
b_mp4_process_sample_s263(bmp4_player_t player, b_mp4_track *track, batom_t atom, const bmp4_sample_s263 *s263)
{
	batom_accum_t dst = player->accum_dest;

	BSTD_UNUSED(s263);
	BDBG_ASSERT(track);
	BDBG_ASSERT(s263);
	BDBG_ASSERT(batom_accum_len(dst)==0);

	if(track->next_sample.syncsample) {
		batom_accum_add_range(dst, track->hdr_buf, track->hdr_len);
		batom_accum_add_atom(dst, atom);
		batom_release(atom);
		return batom_from_accum(dst, NULL, NULL);
	} else {
		return atom;
	}
}

static batom_t
b_mp4_process_sample_avc(bmp4_player_t player, const b_mp4_track *track, batom_t atom, const bmp4_sample_avc *avc)
{
	batom_accum_t src = player->accum_src;
	batom_accum_t dst = player->accum_dest;
	batom_cursor cursor;
	batom_t result = NULL;

	BDBG_ASSERT(track);
	BDBG_ASSERT(avc);
	BDBG_ASSERT(batom_accum_len(src)==0);
	BDBG_ASSERT(batom_accum_len(dst)==0);

	batom_accum_add_atom(src, atom);
	batom_release(atom);
	batom_cursor_from_accum(&cursor, src);

	if(track->next_sample.syncsample && track->hdr_len) {
		batom_accum_add_range(dst, track->hdr_buf, track->hdr_len);
	}

	for(;;)  {
		size_t nal_len;
		batom_cursor nal;
		size_t data_len;


        nal_len = batom_cursor_vword_be(&cursor, avc->meta.nalu_len);
		if(BATOM_IS_EOF(&cursor)) {
			break;
		}
		BATOM_CLONE(&nal, &cursor);
		data_len = batom_cursor_skip(&cursor, nal_len);
		if(data_len!=nal_len || nal_len==0) {
			BDBG_WRN(("b_mp4_process_sample_avc: %#lx invalid nal_len %u:%u", (unsigned long)player, (unsigned)nal_len, (unsigned)data_len));
            b_mp4_player_data_error(player);
			batom_accum_clear(dst);
            result = batom_empty(player->factory, NULL, NULL);
			goto done;
		}
		batom_accum_add_vec(dst, &bmedia_nal_vec);
		batom_accum_append(dst, src, &nal, &cursor);
	}
	result = batom_from_accum(dst, NULL, NULL);
done:
	batom_accum_clear(src);
	return result;
}

static batom_t
b_mp4_process_sample_hevc(bmp4_player_t player, const b_mp4_track *track, batom_t atom, const bmp4_sample_hevc *hevc)
{
    batom_accum_t src = player->accum_src;
    batom_accum_t dst = player->accum_dest;
    batom_cursor cursor;
    batom_t result = NULL;

    BDBG_ASSERT(track);
    BDBG_ASSERT(hevc);
    BDBG_ASSERT(batom_accum_len(src)==0);
    BDBG_ASSERT(batom_accum_len(dst)==0);

    batom_accum_add_atom(src, atom);
    batom_release(atom);
    batom_cursor_from_accum(&cursor, src);

    if(track->next_sample.syncsample && track->hdr_len) {
        batom_accum_add_range(dst, track->hdr_buf, track->hdr_len);
    }

    for(;;)  {
        size_t nal_len;
        batom_cursor nal;
        size_t data_len;

        nal_len = batom_cursor_vword_be(&cursor, hevc->meta.lengthSize);
        if(BATOM_IS_EOF(&cursor)) {
            break;
        }
        BATOM_CLONE(&nal, &cursor);
        data_len = batom_cursor_skip(&cursor, nal_len);
        if(data_len!=nal_len || nal_len==0) {
            BDBG_WRN(("b_mp4_process_sample_hevc: %#lx invalid nal_len %u:%u", (unsigned long)player, (unsigned)nal_len, (unsigned)data_len));
            b_mp4_player_data_error(player);
            batom_accum_clear(dst);
            result = batom_empty(player->factory, NULL, NULL);
            goto done;
        }
        batom_accum_add_vec(dst, &bmedia_nal_vec);
        batom_accum_append(dst, src, &nal, &cursor);
    }
    result = batom_from_accum(dst, NULL, NULL);
done:
    batom_accum_clear(src);
    return result;
}


#if 0
static bool b_mp4_process_sample_mjpb_add_marker(bmp4_player_t player, uint32_t offset, const batom_vec *vec)
{
    size_t size;
    batom_cursor end, begin;

    batom_cursor_from_accum(&begin, player->accum_src);
    batom_cursor_skip(&begin, offset);
    BATOM_CLONE(&end, &begin);
    size = batom_cursor_uint16_be(&end);
    if(BATOM_IS_EOF(&end)) {
        goto error;
    }
    if(size!=batom_cursor_skip(&end, size)) {
        goto error;
    }
    batom_accum_add_vec(player->accum_dest, vec);
    batom_accum_append(player->accum_dest, player->accum_src, &begin, &end);
    return true;
error:
    return false;
}

static batom_t 
b_mp4_process_sample_mjpb(bmp4_player_t player, b_mp4_track *track, batom_t atom, const bmp4_sample_mjpeg *mjpeg)
{
    uint32_t data;
    uint32_t field_size;
    uint32_t padded_field_size;
    uint32_t offset_to_next_field;
    uint32_t quantization_table_offset;
    uint32_t huffman_table_offset;
    uint32_t start_of_frame_offset;
    uint32_t start_of_scan_offset;
    uint32_t start_of_data_offset;
    static const uint8_t zero_byte=0;
    batom_cursor cursor;

    BSTD_UNUSED(player);
    BSTD_UNUSED(mjpeg);
    batom_accum_clear(player->accum_src);
    batom_accum_add_atom(player->accum_src, atom);
    batom_release(atom);
    batom_cursor_from_accum(&cursor, player->accum_src);
    data = batom_cursor_uint32_be(&cursor);
    if(BATOM_IS_EOF(&cursor) || data != 0) {
        goto error;
    }
    data = batom_cursor_uint32_be(&cursor);
    if(BATOM_IS_EOF(&cursor) || data != BMP4_TYPE('m','j','p','g')) {
        goto error;
    }
    field_size = batom_cursor_uint32_be(&cursor);
    padded_field_size = batom_cursor_uint32_be(&cursor);
    offset_to_next_field = batom_cursor_uint32_be(&cursor);
    quantization_table_offset = batom_cursor_uint32_be(&cursor);
    huffman_table_offset = batom_cursor_uint32_be(&cursor);
    start_of_frame_offset = batom_cursor_uint32_be(&cursor);
    start_of_scan_offset = batom_cursor_uint32_be(&cursor);
    start_of_data_offset = batom_cursor_uint32_be(&cursor);
    if(BATOM_IS_EOF(&cursor)) {
        goto error;
    }

    BDBG_MSG(("b_mp4_process_sample_mjpb:%p:%p field_size:%u padded_field_size:%u offset_to_next_field:%u quantization_table_offset:%u huffman_table_offset:%u start_of_frame_offset:%u start_of_scan_offset:%u start_of_data_offset:%u", player, track, field_size, padded_field_size, offset_to_next_field, quantization_table_offset, huffman_table_offset, start_of_data_offset, start_of_scan_offset, start_of_data_offset));
    batom_accum_clear(player->accum_dest);
    batom_accum_add_vec(player->accum_dest, &bmedia_jpeg_stream_header);

    if(!b_mp4_process_sample_mjpb_add_marker(player, quantization_table_offset, &bmedia_jpeg_dqt)) {
        goto error;
    }
    if(!b_mp4_process_sample_mjpb_add_marker(player, huffman_table_offset, &bmedia_jpeg_dht)) {
        goto error;
    }
    if(!b_mp4_process_sample_mjpb_add_marker(player, start_of_frame_offset, &bmedia_jpeg_sof)) {
        goto error;
    }
    if(!b_mp4_process_sample_mjpb_add_marker(player, start_of_scan_offset, &bmedia_jpeg_sos)) {
        goto error;
    }
    if(start_of_data_offset<field_size) {
        batom_cursor begin;
        size_t left;

        batom_cursor_from_accum(&begin, player->accum_src);
        batom_cursor_skip(&begin, start_of_data_offset);
        BATOM_CLONE(&cursor, &begin);
        for(left = field_size - start_of_data_offset;left>0;left--) {
            int next;
            BATOM_NEXT(next, &cursor);
            if(next==BATOM_EOF) {
                goto error;
            }
            if(next==0xFF) {
                batom_accum_append(player->accum_dest, player->accum_src, &begin, &cursor);
                batom_accum_add_range(player->accum_dest, &zero_byte, 1);
                BATOM_CLONE(&begin, &cursor);
            }
        }
        batom_accum_append(player->accum_dest, player->accum_src, &begin, &cursor);
    }

    batom_accum_add_vec(player->accum_dest, &bmedia_jpeg_eoi);
    batom_accum_clear(player->accum_src);

    return batom_from_accum(player->accum_dest, NULL, NULL);

error:
    batom_accum_clear(player->accum_src);
    return NULL;
}
#endif /* 0 -> MJPB -> JPEG conversion */

static batom_t
b_mp4_process_sample_mjpeg(bmp4_player_t player, b_mp4_track *track, batom_t atom, const bmp4_sample_mjpeg *mjpeg)
{
    const size_t header_len = BMEDIA_FIELD_LEN("size", uint32_t) + BMEDIA_FIELD_LEN("type", uint16_t);
    uint8_t *hdr = track->hdr_buf;
    unsigned type;
    batom_t frame;
    batom_vec vec;

    BDBG_ASSERT(track);
    BSTD_UNUSED(player);

    BKNI_Memcpy(hdr, bmedia_frame_bcmv.base, bmedia_frame_bcmv.len);
    hdr += bmedia_frame_bcmv.len;
    switch(mjpeg->type) {
    default:
    case BMP4_SAMPLE_JPEG: type = 0; break;
    case BMP4_SAMPLE_MJPA: type = 1; break;
    case BMP4_SAMPLE_MJPB: type = 2; break;
    }
#if 0
    /* batom_dump(atom,"MJPB"); */
    atom = b_mp4_process_sample_mjpb(player, track, atom, mjpeg);
#endif


    if(1) { /* not RAW */
        B_MEDIA_SAVE_UINT32_BE(hdr, batom_len(atom)+header_len+bmedia_frame_bcmv.len); /* size */
        B_MEDIA_SAVE_UINT16_BE(hdr+BMEDIA_FIELD_LEN("size",uint32_t), type); /* type */;
        BATOM_VEC_INIT(&vec, track->hdr_buf, header_len + bmedia_frame_bcmv.len);
        frame = batom_from_range_and_atom(track->hdr_buf, header_len + bmedia_frame_bcmv.len, atom, NULL, NULL);
        batom_release(atom);
    } else {
        frame = atom;
    }
    return frame;
}

static batom_t
b_mp4_process_sample_generic(bmp4_player_t player, b_mp4_track *track, batom_t atom)
{
    BSTD_UNUSED(player);
    BSTD_UNUSED(track);
    return atom;
}

static batom_t
b_mp4_player_make_pes_packet(bmp4_player_t player, b_mp4_track *track, batom_t atom, bool with_pts)
{
    batom_t pes_atom=NULL;
    uint8_t pes_id=track->pes_id;

    if(pes_id>=2) { /* not ES */
        bmedia_pes_info pes_info;
        BMEDIA_PES_INFO_INIT(&pes_info, pes_id);
        if(with_pts) {
            BMEDIA_PES_SET_PTS(&pes_info, bmedia_time2pts(track->next_sample.time, player->time_scale));
        }
        if(player->config.max_pes_size != 0) {
            batom_accum_clear(player->accum_src);
            batom_accum_add_atom(player->accum_src, atom);
            batom_release(atom);
            pes_atom = bmedia_pes_subdivide_packet(player->accum_src, &pes_info, player->accum_dest, player->pes_buf, sizeof(player->pes_buf), player->config.max_pes_size);
        } else {
            size_t hdr_len;
            size_t payload_len = batom_len(atom);
            if(track->bounded_pes && payload_len > B_MEDIA_MAX_PES_SIZE) {
                batom_accum_clear(player->accum_src);
                batom_accum_add_atom(player->accum_src, atom);
                batom_release(atom);
                pes_atom = bmedia_pes_subdivide_packet(player->accum_src, &pes_info, player->accum_dest, player->pes_buf, sizeof(player->pes_buf), B_MEDIA_MAX_PES_SIZE);
            } else {
                hdr_len = bmedia_pes_header_init(player->pes_buf, payload_len, &pes_info);
                pes_atom = batom_from_range_and_atom(player->pes_buf, hdr_len, atom, NULL, NULL);
                batom_release(atom);
            }
        }
    } else {
        pes_atom = atom;
    }
    return pes_atom;
}

static batom_t
b_mp4_player_continue_sample(bmp4_player_t player, b_mp4_track *track)
{
    batom_t atom;

    BDBG_MSG_TRACE(("b_mp4_player_continue_sample:%p %p", (void *)player, (void *)track));

    switch(track->info.sample_info.entries[0]->sample_type) {
    case bmp4_sample_type_twos:
    case bmp4_sample_type_qt_ima_adpcm:
    case bmp4_sample_type_qt_ima4_adpcm:
        atom = b_mp4_continue_sample_pcm(player, track);
        break;
    default:
        atom = NULL;
        break;
    }
    BDBG_MSG_TRACE(("b_mp4_player_continue_sample:%p %p->%p", (void *)player, (void *)track, (void *)atom));
    if(atom==NULL) {
        return NULL;
    }
    return b_mp4_player_make_pes_packet(player, track, atom, false);
}

#define B_MP4_PLAYER_SAVE_FRAMES 0

#if B_MP4_PLAYER_SAVE_FRAMES
#include <stdio.h>
#endif


static batom_t
b_mp4_player_process_sample(bmp4_player_t player, b_mp4_track *track, batom_t atom, bool *partial_data)
{
    batom_t pes_atom=NULL;
    uint8_t pes_id=track->pes_id;
#if B_MP4_PLAYER_SAVE_FRAMES
    char fname[64];
    uint8_t data_buf[128];
    batom_cursor data_cursor;
    static int frame_no=0;
    FILE *fout;
    frame_no ++;
#endif

    *partial_data = false;
    BDBG_MSG_TRACE(("b_mp4_player_process_sample:%p %p", (void *)player, (void *)track));
    if(player->stream.decrypt_callback){
        batom_cursor cursor;
        bmp4_player_drm_info drm_info;

        batom_cursor_from_atom(&cursor, atom);
        drm_info.type = bmp4_player_drm_info_frame;
        drm_info.data.frame.sample_no = 0;
        if(track->fragment_valid) {
            drm_info.data.frame.sample_no = track->fragment.run_state.sample_no;
        }

        player->stream.decrypt_callback(player->stream.cntx, &cursor, NULL, &drm_info, track->info.trackheader.track_ID);
    }
#if B_MP4_PLAYER_SAVE_FRAMES
    {
        BKNI_Snprintf(fname, sizeof(fname), "frame_%u_%u.in", track->info.trackheader.track_ID, frame_no);
        BDBG_LOG(("writing %s, %u bytes", fname, batom_len(atom)));
        fout = fopen(fname,"wb");
        if(fout) {
            batom_cursor_from_atom(&data_cursor, atom);
            do {
                size_t size = batom_cursor_copy(&data_cursor, data_buf, sizeof(data_buf));
                fwrite(data_buf, 1, size, fout);
            } while(!BATOM_IS_EOF(&data_cursor));
            fclose(fout);
        }
    }
#endif



    if(pes_id!=1) { /* not RAW */
        switch(track->info.sample_info.entries[0]->sample_type) {
        case bmp4_sample_type_avc:
            atom = b_mp4_process_sample_avc(player, track, atom, &track->info.sample_info.entries[0]->codec.avc);
            break;
        case bmp4_sample_type_hevc:
            atom = b_mp4_process_sample_hevc(player, track, atom, &track->info.sample_info.entries[0]->codec.hevc);
            break;
        case bmp4_sample_type_mp4a:
            atom = b_mp4_process_sample_mp4a(player, track, atom, &track->info.sample_info.entries[0]->codec.mp4a);
            break;
        case bmp4_sample_type_als:
            atom = b_mp4_process_sample_als(player, track, atom, &track->info.sample_info.entries[0]->codec.mp4a);
            break;
        case bmp4_sample_type_mp4v:
            atom = b_mp4_process_sample_mp4v(player, track, atom, &track->info.sample_info.entries[0]->codec.mp4v);
            break;
        case bmp4_sample_type_s263:
            atom = b_mp4_process_sample_s263(player, track, atom, &track->info.sample_info.entries[0]->codec.s263);
            break;
        case bmp4_sample_type_ac3:
        case bmp4_sample_type_eac3:
        case bmp4_sample_type_dts:
            atom = b_mp4_process_sample_generic(player, track, atom);
            break;
        case bmp4_sample_type_samr:
        case bmp4_sample_type_sawb:
        case bmp4_sample_type_sawp:
            atom = b_mp4_process_sample_amr(player, track, atom, &track->info.sample_info.entries[0]->codec.amr);
            break;
        case bmp4_sample_type_twos:
        case bmp4_sample_type_qt_ima_adpcm:
        case bmp4_sample_type_qt_ima4_adpcm:
            atom = b_mp4_process_sample_pcm(player, track, atom);
            if(atom==NULL) {
                *partial_data = true;
            }
            break;
        case bmp4_sample_type_mjpeg:
            atom = b_mp4_process_sample_mjpeg(player, track, atom, &track->info.sample_info.entries[0]->codec.mjpeg);
            break;
        default:
            atom = b_mp4_process_sample_generic(player, track, atom);
            break;
        }
    }
#if B_MP4_PLAYER_SAVE_FRAMES
    if(atom) {
        BKNI_Snprintf(fname, sizeof(fname), "frame_%u_%u.out", track->info.trackheader.track_ID, frame_no);
        BDBG_LOG(("writing %s, %u bytes", fname, batom_len(atom)));
        fout = fopen(fname,"wb");
        if(fout) {
            batom_cursor_from_atom(&data_cursor, atom);
            do {
                size_t size = batom_cursor_copy(&data_cursor, data_buf, sizeof(data_buf));
                fwrite(data_buf, 1, size, fout);
            } while(!BATOM_IS_EOF(&data_cursor));
            fclose(fout);
        }
    }
#endif
    if(atom) {
        pes_atom = b_mp4_player_make_pes_packet(player, track, atom, true);
    }
#if 0
    {
        batom_factory_stats factory_stats;
        batom_factory_get_stats(player->factory, &factory_stats);
        BDBG_WRN(("demux_status: atoms[live:%u allocated:%u freed:%u] alloc[pool:%u/%u/%u arena:%u/%u/%u alloc:%u/%u]", factory_stats.atom_live, factory_stats.atom_allocated, factory_stats.atom_freed, factory_stats.alloc_pool, factory_stats.free_pool, factory_stats.full_pool, factory_stats.alloc_arena, factory_stats.free_arena, factory_stats.full_arena, factory_stats.alloc_alloc, factory_stats.free_alloc));
    }
#endif
    return pes_atom;
}


static void
b_mp4_player_read_cont(void *cntx, batom_t atom, bfile_buffer_result result)
{
    bmp4_player_t player = cntx;
    b_mp4_track *track;
    bmedia_player_entry entry;

    BDBG_OBJECT_ASSERT(player, bmp4_player_t);
    BDBG_ASSERT(player->current_track);
    bmedia_player_init_entry(&entry);
    track = player->current_track;
    entry.timestamp = track->next_sample.time;
    entry.start = track->next_sample.offset;
    if(atom && result==bfile_buffer_result_ok) {
        bool partial_data;
        entry.atom = b_mp4_player_process_sample(player, track, atom, &partial_data);
        if(partial_data) {
            track->next_sample_valid = false;
            track->next_sample_valid = false;
            player->current_track = NULL;
            result = bfile_buffer_result_no_data;
        }
    } else {
        if(atom) {
            batom_release(atom);
        }
        entry.atom = NULL;
        if(result!=bfile_buffer_result_no_data) {
            BDBG_WRN(("b_mp4_player_read_cont: read failed %p (" B_OFFT_FMT ":%u)", (void *)player, B_OFFT_ARG(track->next_sample.offset), (unsigned)track->next_sample.len));
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
            b_mp4_player_data_error(player);
            break;
        default:
            entry.type = bmedia_player_entry_type_error;
            b_mp4_player_data_error(player);
            break;
        }
        entry.length = 0;
    }
    if(entry.type != bmedia_player_entry_type_no_data) {
        track->next_sample_valid = false;
        player->current_track = NULL;
    }
    BDBG_ASSERT(player->config.atom_ready);
    player->config.atom_ready(player->config.cntx, &entry);
    return;
}

static batom_t
b_mp4_player_load_sample(bmp4_player_t player, b_mp4_track *track, bfile_buffer_result *result)
{
    batom_t atom;

    BDBG_MSG(("b_mp4_player_load_sample: %p %p:%s loading from " B_OFFT_FMT ":%u", (void *)player, (void *)track,
          track->info.sample_info.entries[0]->sample_type==bmp4_sample_type_avc?"AVC":
          track->info.sample_info.entries[0]->sample_type==bmp4_sample_type_mp4v?"MP4V":
          track->info.sample_info.entries[0]->sample_type==bmp4_sample_type_s263?"S263":
          track->info.sample_info.entries[0]->sample_type==bmp4_sample_type_mp4a?"MP4A":"OTHER",
          B_OFFT_ARG(track->next_sample.offset), (unsigned)track->next_sample.len));

    BDBG_ASSERT(track->info.sample_info.entry_count==1); /* it was previously verified */
    atom = bfile_buffer_read(player->config.buffer, track->next_sample.offset, track->next_sample.len, result);
    if(*result == bfile_buffer_result_ok && atom) {
        bool partial_data;
        atom = b_mp4_player_process_sample(player, track, atom, &partial_data);
        if(partial_data) {
            track->next_sample_valid = false;
            *result = bfile_buffer_result_no_data;
        }
        return atom;
    } else {
        if(atom) {
            batom_release(atom);
        }
        if(*result != bfile_buffer_result_no_data) {
            BDBG_WRN(("b_mp4_player_load_sample: %p error %u (" B_OFFT_FMT ":%u)", (void *)player, (unsigned)*result, B_OFFT_ARG(track->next_sample.offset), (unsigned)track->next_sample.len));
            b_mp4_player_data_error(player);
        }
        return NULL;
    }
}

static b_mp4_track  *
b_mp4_player_get_video_track(bmp4_player_t player)
{
    b_mp4_track *track;
    for(track = BLST_SQ_FIRST(&player->movie.tracks); track ; track=BLST_SQ_NEXT(track, next)) {
        switch(track->info.sample_info.entries[0]->sample_type) {
        case bmp4_sample_type_avc:
        case bmp4_sample_type_hevc:
        case bmp4_sample_type_mp4v:
        case bmp4_sample_type_s263:
        case bmp4_sample_type_mjpeg:
            if(player->fragmented || track->stream) {
                 return track;
            }
            break;
        default:
            break;
        }
    }
    return false;
}

int
b_mp4_player_stream_sample_next(b_mp4_track *track, bmp4_sample *sample)
{
    size_t max_sample_count;

    switch(track->info.sample_info.entries[0]->sample_type) {
    case bmp4_sample_type_twos:
    case bmp4_sample_type_qt_ima_adpcm:
    case bmp4_sample_type_qt_ima4_adpcm:
        max_sample_count = 256;
        break;
    default:
        max_sample_count = 1;
        break;
    }

    return bmp4_stream_sample_next(track->stream, sample, max_sample_count);
}

static b_mp4_track *
b_mp4_player_next_backward(bmp4_player_t player)
{
    b_mp4_track *track;

    BDBG_CASSERT(B_MP4_PLAYER_REW_STACK > 1);
    BDBG_CASSERT(B_MP4_PLAYER_REW_STEP > 33);

    track = b_mp4_player_get_video_track(player);
    if(!track) {
        return NULL;
    }
    for(;;) {
        bmedia_player_pos sync_pos;
        bmedia_player_pos track_pos;
        unsigned stack_next;
        bool endofstream;

        BDBG_MSG_TRACE(("b_mp4_player_next_backward: %#lx stack_next:%u", (unsigned long)player, (unsigned)player->rew_stack.stack_next));
        if(player->rew_stack.stack_next>0) {
            player->rew_stack.stack_next--;
            /* 1. find sample behing current position */
            sync_pos = player->rew_stack.stack[player->rew_stack.stack_next].time;
            BDBG_MSG_TRACE(("b_mp4_player_next_backward: %#lx %u:%u", (unsigned long)player, (unsigned)sync_pos, (unsigned)player->status.position));
            if(sync_pos>player->status.position) {
                continue;
            }
            track->next_sample = player->rew_stack.stack[player->rew_stack.stack_next];
            track_pos = sync_pos - (unsigned)(-player->status.direction);
            /* check for wraparound (player->direction is negative) */
            if(track_pos>sync_pos) {
                track_pos = 0;
            }
            player->status.position = track_pos;
            return track;
        }
        /* 2. jump backward */
        sync_pos = player->rew_stack.position;
        if(sync_pos==0) {
            /* reached beginning of the stream */
            BDBG_MSG(("b_mp4_player_next_backward:%#lx beginning of file", (unsigned long)player));
            BKNI_Memset(&track->next_sample, 0, sizeof(track->next_sample));
            track->next_sample.endofstream = true;
            return track;
        }
        if(sync_pos > B_MP4_PLAYER_REW_STEP) {
            sync_pos -= B_MP4_PLAYER_REW_STEP;
        } else {
            sync_pos = 0;
        }
        track_pos = bmp4_stream_sample_seek(track->stream, sync_pos, &endofstream);
        if(track_pos==BMEDIA_PLAYER_INVALID || endofstream)  {
            BDBG_MSG(("b_mp4_player_next_backward:%#lx beginning of file (can't seek to %u)", (unsigned long)player, (unsigned)sync_pos));
            return NULL;
        }
        if(track_pos>=player->status.position) {
            /* if actual seek time after current position, it means that we didn't jump far enough, so continue jumping */
            player->rew_stack.position = sync_pos;
            continue;
        }
        BDBG_MSG(("b_mp4_player_next_backward: %#lx scan start track_pos:%u player->position:%u", (unsigned long)player, (unsigned)track_pos, (unsigned)player->status.position));
        /* 3. scan stream forward and save entries into stack */
        for(stack_next=0;;) {
            int rc = b_mp4_player_stream_sample_next(track, &player->rew_stack.stack[stack_next]);
            BDBG_MSG_TRACE(("b_mp4_player_next_backward: %#lx next_sample rc:%u %s", (unsigned long)player, rc, player->rew_stack.stack[stack_next].endofstream?"endofstream":""));
            if(rc<0 || player->rew_stack.stack[stack_next].endofstream) {
                break;
            }
            if(!player->rew_stack.stack[stack_next].syncsample) {
                continue;
            }
            sync_pos = player->rew_stack.stack[stack_next].time ;
            BDBG_MSG_TRACE(("b_mp4_player_next_backward: %#lx sample:%u limit:%u", (unsigned long)player, (unsigned)sync_pos, (unsigned)player->status.position));
            if(sync_pos > player->status.position) {
                /* we scanned past of current play position */
                break;
            }
            stack_next++;
            if(stack_next<B_MP4_PLAYER_REW_STACK) {
                continue;
            }
            /* entire stack filled with data, remove first entry */
            BKNI_Memmove(&player->rew_stack.stack[0], &player->rew_stack.stack[1], (B_MP4_PLAYER_REW_STACK-1)*sizeof(player->rew_stack.stack[0]));
            stack_next = B_MP4_PLAYER_REW_STACK-1;
        }
        BDBG_MSG(("b_mp4_player_next_backward: %#lx scan done stack_next:%u sync_pos:%u", (unsigned long)player, (unsigned)stack_next, (unsigned)sync_pos));
        player->rew_stack.stack_next = stack_next;
        player->rew_stack.position = sync_pos;
    }
}

static b_mp4_track *
b_mp4_player_next_forward(bmp4_player_t player)
{
    b_mp4_track *track;

    track = b_mp4_player_get_video_track(player);
    if(track) {
        int rc;

        for(;;) {
            rc = b_mp4_player_stream_sample_next(track, &track->next_sample);
            if(rc<0) {
                return NULL;
            }
            if(track->next_sample.endofstream) {
                break;
            }
            if(track->next_sample.syncsample && track->next_sample.time>=player->status.position) {
                player->status.position=track->next_sample.time + player->status.direction;
                break;
            }
        }
    }
    return track;
}

static b_mp4_track *
b_mp4_player_next_play(bmp4_player_t player)
{
    b_mp4_track *track;
    b_mp4_track *min_track;
    b_mp4_track *eof_track;

    BDBG_MSG_TRACE(("b_mp4_player_next_play:%p", (void *)player));
    for(eof_track=NULL,min_track=NULL, track = BLST_SQ_FIRST(&player->movie.tracks); track ; track=BLST_SQ_NEXT(track, next)) {
        BDBG_MSG_TRACE(("b_mp4_player_next_play:%p track:%p", (void *)player, (void *)track));
        if(!track->stream) {
            continue;
        }
        if(!track->next_sample_valid) {
            int rc;
            BDBG_MSG_TRACE(("b_mp4_player_next_play:%p next:%p", (void *)player, (void *)track));
            rc = b_mp4_player_stream_sample_next(track, &track->next_sample);
            track->next_sample_valid = (rc==0);
            if(rc!=0) {
                return NULL;
            }
        }
        if(track->next_sample_valid) {
            BDBG_MSG_TRACE(("b_mp4_player_next_play: %#lx stream:%u" B_MP4_TYPE_FORMAT " %u(%#lx) time %u", (unsigned long)player, track->info.trackheader.track_ID, B_MP4_TYPE_ARG(track->info.sample_info.entries[0]->type), track->info.trackheader.track_ID, (unsigned long)track, (unsigned)track->next_sample.time));
            if(track->next_sample.endofstream==false)  {
                if(min_track==NULL) {
                    min_track = track;
                } else if(track->next_sample.time < min_track->next_sample.time) {
                    min_track = track;
                }
                player->status.position=min_track->next_sample.time;
            } else if(eof_track==NULL) {
                eof_track = track;
            }
        }
    }
    if(min_track) {
        return min_track;
    } else {
        return eof_track;
    }
}

static int
b_mp4_player_track_fragmented_read(bmp4_player_t player, b_mp4_fragment_track *track, const bmp4_trackextendsbox *track_extends, bool *end_of_file)
{
    int rc=0;
    bool end_of_data;

    *end_of_file = false;
    for(;;) {

        if(!track->movie_fragment_valid) {
            bmp4_movie_fragment_header movie_fragment_header;
            b_mp4_active_box movie_fragment_header_box;
            bmp4_box mdat;
            size_t header_size;

            BDBG_MSG_TRACE(("b_mp4_player_track_fragmented_read:%#lx reading movie_fragment", (unsigned long)track));
            rc = b_mp4_active_box_next(&track->root, &track->cache, &track->movie_fragment, &end_of_data);
            if(rc<0) {
                if(end_of_data) {
                    *end_of_file = true;
                    break;
                } else {
                    break;
                }
            }

            BDBG_MSG_TRACE(("b_mp4_player_track_fragmented_read:%#lx reading movie_fragment_header", (unsigned long)track));
            b_mp4_active_box_init(&movie_fragment_header_box, BMP4_MOVIE_FRAGMENT_HEADER);
            rc = b_mp4_active_box_next(&track->movie_fragment, &track->cache, &movie_fragment_header_box, &end_of_data);
            if(rc<0) {
                break;
            }
            if(!bmp4_parse_movie_fragment_header(&track->cache.cursor, &movie_fragment_header)) {
                return -1;
            }
            BDBG_MSG_TRACE(("b_mp4_player_track_fragmented_read:%#lx reading movie_fragment_data", (unsigned long)track));
            rc = b_mp4_active_box_peek(&track->root, &track->cache, &track->movie_fragment_data, &end_of_data, &mdat, &header_size);
            if(rc==0 && mdat.type == track->movie_fragment_data.type) {
                BDBG_MSG_TRACE(("b_mp4_player_track_fragmented_read:%#lx movie_fragment_data at:%u", (unsigned long)track, (unsigned)track->movie_fragment_data.start));
                track->movie_fragment_data_valid = true;
            } else {
                track->movie_fragment_data_valid = false;
            }
            track->state.next.run_offset = track->state.current.run_offset = 0;
            track->movie_fragment_valid = true;
        }
        if(!track->fragment_header_valid) {
            b_mp4_active_box track_fragment_header;

            BDBG_MSG_TRACE(("b_mp4_player_track_fragmented_read:%#lx reading track_fragment", (unsigned long)track));
            rc = b_mp4_active_box_next(&track->movie_fragment, &track->cache, &track->track_fragment, &end_of_data);
            if(rc<0) {
                if(end_of_data) {
                    track->movie_fragment_valid = false; /* look for the next fragment */
                    continue;
                } else {
                    break;
                }
            }

            b_mp4_active_box_init(&track_fragment_header, BMP4_TRACK_FRAGMENT_HEADER);
            rc = b_mp4_active_box_next(&track->track_fragment, &track->cache, &track_fragment_header, &end_of_data);
            if(rc<0) {
                return rc;
            }
            if(!bmp4_parse_track_fragment_header(&track->cache.cursor, &track->track_fragment_header)) {
                return -1;
            }
            BDBG_MSG_TRACE(("b_mp4_player_track_fragmented_read:%#lx found track %u:%u", (unsigned long)track, track->track_fragment_header.track_ID, track_extends->track_ID));
            if(track->track_fragment_header.track_ID != track_extends->track_ID) {
                continue; /* advance to the next fragment */
            }
            track->fragment_header_valid = true;
            if(player->stream.decrypt_callback){
                bmp4_player_drm_info drm_info;

                drm_info.type = bmp4_player_drm_info_track_fragment;
                drm_info.data.track_fragment.offset = track->track_fragment.start;
                drm_info.data.track_fragment.length  = track->track_fragment.len;
                drm_info.data.track_fragment.buffer = player->fragment.buffer;

                player->stream.decrypt_callback(player->stream.cntx, NULL, NULL, &drm_info, track->track_fragment_header.track_ID);
            }
        }
        if(!track->fragment_run_valid) {
            BDBG_MSG_TRACE(("b_mp4_player_track_fragmented_read:%#lx reading track_fragment_run", (unsigned long)track));
            rc = b_mp4_active_box_next(&track->track_fragment, &track->cache, &track->track_fragment_run, &end_of_data);
            if(rc<0) {
                if(end_of_data) {
                    track->fragment_header_valid = false; 
                    continue;
                } else {
                    break;
                }
            }
            if(!bmp4_parse_track_fragment_run_header(&track->cache.cursor, &track->run_header)) {
                return -1;
            }
            bmp4_init_track_fragment_run_state(&track->run_state);
            track->fragment_run_valid = true;
        }
        if(track->run_state.sample_no >= track->run_header.sample_count) {
            track->state.current = track->state.next;
            track->fragment_run_valid = false;
            continue;
        }
        if(!bmp4_parse_track_fragment_run_sample(&track->cache.cursor, &track->track_fragment_header, &track->run_header, track_extends, &track->run_state, &track->next_sample)) {
            track->fragment_run_valid = false;
            continue;
        }
        BDBG_MSG_TRACE(("b_mp4_player_track_fragmented_read:%#lx track_ID:%u sample.offset:%u %s", (unsigned long)track, track_extends->track_ID, (unsigned)track->next_sample.offset, track->run_header.validate.data_offset?"run_header.data_offset":""));
        if(track->run_header.validate.data_offset) {
            track->next_sample.offset += track->run_header.data_offset;
        } else {
            track->next_sample.offset += track->state.current.run_offset;
        }
        track->state.next.run_offset = track->next_sample.offset + track->next_sample.size;
        BDBG_MSG_TRACE(("b_mp4_player_track_fragmented_read:%#lx track_ID:%u run.offset:%u %s %s %u %u", (unsigned long)track, track_extends->track_ID, (unsigned)track->next_sample.offset, track->track_fragment_header.validate.base_data_offset?"track_fragment_header.base_data_offset":"", track->movie_fragment_data_valid?"MDAT.offset":"", (unsigned)track->movie_fragment_data.start, (unsigned)(track->movie_fragment.start-track->movie_fragment.header_len)));
        if(track->track_fragment_header.validate.base_data_offset) {
            track->next_sample.offset += track->track_fragment_header.base_data_offset;
        } else {
            if(track->movie_fragment_data_valid && !track->run_header.validate.data_offset)  {
                track->next_sample.offset += track->movie_fragment_data.start;
            } else {
                track->next_sample.offset += track->movie_fragment.start - track->movie_fragment.header_len;
            }
        }
        track->next_sample.time += track->state.current.run_start_time;
        track->state.next.run_start_time = track->next_sample.time + track->next_sample.duration;
        break;
    }

    BDBG_MSG(("b_mp4_player_track_fragmented_read:%#lx track_ID:%u sample offset:%u time:%u duration:%u size:%u flags:%#x composition_time_offset:%d %s", (unsigned long)track, (unsigned)track_extends->track_ID, (unsigned)track->next_sample.offset, (unsigned)track->next_sample.time, track->next_sample.duration, track->next_sample.size, track->next_sample.flags, (int)track->next_sample.composition_time_offset, *end_of_file?"EOF":""));
    return rc;
}

static int
b_mp4_player_track_fragmented_next(bmp4_player_t player, b_mp4_track *track, bmp4_track_fragment_run_sample *sample, bool *end_of_file)
{
    int rc;
    if(track->fragment.next_sample_valid) {
        *end_of_file = false;
        rc = 0;
    } else {
        rc = b_mp4_player_track_fragmented_read(player, &track->fragment, &track->track_extends, end_of_file);
        track->fragment.next_sample_valid = rc == 0;
    }
    *sample = track->fragment.next_sample;
    sample->time = (1000*(sample->time + (int32_t)sample->composition_time_offset)) / track->info.mediaheader.timescale;
    BDBG_MSG(("b_mp4_player_track_fragmented_next: %#lx track_ID:%u time:%u %s", (unsigned long)track, track->info.trackheader.track_ID, (unsigned)sample->time, *end_of_file?"EOF":""));
    return rc;
}

static void
b_mp4_player_track_fragmented_advance(b_mp4_track *track)
{
        track->fragment.next_sample_valid = false;
        return;
}

static b_mp4_track *
b_mp4_player_fragmented_next_play(bmp4_player_t player)
{
    b_mp4_track *track;
    bmp4_track_fragment_run_sample sample, min_sample;
    bool end_of_file;
    b_mp4_track *min_track;
    b_mp4_track *eof_track;
    int rc = 0;


    BKNI_Memset(&min_sample, 0, sizeof(min_sample));
    for(eof_track=NULL, min_track=NULL, track = BLST_SQ_FIRST(&player->movie.tracks); track; track = BLST_SQ_NEXT(track, next)) {
        if(!track->fragment_valid) {
            continue;
        }
        rc = b_mp4_player_track_fragmented_next(player, track, &sample, &end_of_file);
        if(rc<0) {
            if(end_of_file) {
                eof_track = track;
                continue;
            } else {
                break;
            }
        }
        if(min_track==NULL || min_sample.time > sample.time)  {
            min_track = track;
            min_sample = sample;
        }
    }
    if(min_track) {
        BDBG_MSG(("b_mp4_player_fragmented_next: %p time:%u track_ID:%u flags:%#x", (void *)min_track,  (unsigned)min_sample.time,  (unsigned)min_track->info.trackheader.track_ID, (unsigned)min_sample.flags));
        min_track->next_sample.endofstream = false;
        min_track->next_sample.time = min_sample.time;
        min_track->next_sample.offset = min_sample.offset;
        min_track->next_sample.len = min_sample.size;
        min_track->next_sample.syncsample = min_sample.time==0 || !BMP4_SAMPLE_IS_DIFFERENCE_SAMPLE(min_sample.flags);
        player->status.position=min_sample.time;
        b_mp4_player_track_fragmented_advance(min_track);
    } else if(eof_track) {
        eof_track->next_sample.endofstream = true;
    }

    return min_track;
}

static b_mp4_track *
b_mp4_player_fragmented_next_forward(bmp4_player_t player)
{
    b_mp4_track *track;
    bmp4_track_fragment_random_access_payload tfra;
    unsigned index_entry;
    bmedia_player_pos pos;

    track = b_mp4_player_get_video_track(player);
    BDBG_ASSERT(track); /* it was verified in b_mp4_player_fragmented_set_direction */

    if(track->fragment.state.trick.end_of_index) {
        track->next_sample.endofstream = true;
        return track;
    }
    index_entry=track->fragment.state.trick.index_entry;
    /* 1. Read current index entry */
    if(!b_mp4_player_read_track_fragment_random_access_payload(track, index_entry, &tfra)) {
        return NULL;
    }
    pos = (1000*tfra.time) / track->info.mediaheader.timescale;
    pos = pos + player->status.direction;
    /* 2. Find next entry */
    for(;;) {
        bmp4_track_fragment_random_access_payload new_tfra;
        bmedia_player_pos tfra_pos;

        index_entry++;
        if(index_entry>=track->fragment.index.tfra_header.number_of_entry) {
            track->fragment.state.trick.end_of_index = true;
            break;
        }
        if(!b_mp4_player_read_track_fragment_random_access_payload(track, index_entry, &new_tfra)) {
            return NULL;
        }
        tfra_pos = (1000*new_tfra.time) / track->info.mediaheader.timescale;
        BDBG_MSG(("b_mp4_player_fragmented_next_forward:%p %p:(%u) %u:%u", (void *)player, (void *)track, track->info.trackheader.track_ID, (unsigned)tfra_pos, (unsigned)pos));
        if(tfra_pos >= pos ||
           index_entry+1 == track->fragment.index.tfra_header.number_of_entry /* always include last index entry */
           ) {
            BDBG_MSG(("b_mp4_player_fragmented_next_forward:%p %p:(%u) next %u->%u", (void *)player, (void *)track, track->info.trackheader.track_ID, track->fragment.state.trick.index_entry, index_entry));
            track->fragment.state.trick.index_entry=index_entry;
            break;
        }
    }
    /* 3. Seek fragment to destination */
    b_mp4_fragment_track_clear(&track->fragment);
    BDBG_MSG(("b_mp4_player_fragmented_next_forward:%p %p:(%u) %u -> %u", (void *)player, (void *)track, track->info.trackheader.track_ID, (unsigned)pos, (unsigned) tfra.moof_offset));
    b_mp4_active_box_set(&track->fragment.root, tfra.moof_offset, (uint64_t)(-1), 0);
    track->fragment.state.current.run_start_time = tfra.time;
    track->fragment.state.next.run_start_time = tfra.time;

    /* 4. Read fragment */
    {
        int rc;
        bmp4_track_fragment_run_sample sample;
        bool end_of_file;

        rc = b_mp4_player_track_fragmented_next(player, track, &sample, &end_of_file);
        if(rc<0) {
            return NULL;
        }
        BDBG_MSG(("b_mp4_player_fragmented_next_forward: %p time:%u track_ID:%u flags:%#x", (void *)track,  (unsigned)sample.time,  (unsigned)track->info.trackheader.track_ID, (unsigned)sample.flags));
        track->next_sample.endofstream = false;
        track->next_sample.time = sample.time;
        track->next_sample.offset = sample.offset;
        track->next_sample.len = sample.size;
        track->next_sample.syncsample = true;
        player->status.position=sample.time;
        b_mp4_player_track_fragmented_advance(track);
    }
    return track;
}

static b_mp4_track *
b_mp4_player_fragmented_next_backward(bmp4_player_t player)
{
    b_mp4_track *track;
    bmp4_track_fragment_random_access_payload tfra;
    unsigned index_entry;
    bmedia_player_pos pos;

    track = b_mp4_player_get_video_track(player);
    BDBG_ASSERT(track); /* it was verified in b_mp4_player_fragmented_set_direction */

    if(track->fragment.state.trick.end_of_index) {
        track->next_sample.endofstream = true;
        return track;
    }
    index_entry=track->fragment.state.trick.index_entry;
    /* 1. Read current index entry */
    if(!b_mp4_player_read_track_fragment_random_access_payload(track, index_entry, &tfra)) {
        return NULL;
    }
    pos = (1000*tfra.time) / track->info.mediaheader.timescale;
    BDBG_ASSERT(player->status.direction<0);
    if(pos > (unsigned)-player->status.direction) {
        pos = pos + player->status.direction;
    } else {
        pos = 0;
    }
    /* 2. Find next entry */
    for(;;) {
        bmp4_track_fragment_random_access_payload new_tfra;
        bmedia_player_pos tfra_pos;

        if(index_entry<=0) {
            track->fragment.state.trick.end_of_index = true;
            break;
        }
        index_entry --;
        if(!b_mp4_player_read_track_fragment_random_access_payload(track, index_entry, &new_tfra)) {
            return NULL;
        }
        tfra_pos = (1000*new_tfra.time) / track->info.mediaheader.timescale;
        BDBG_MSG(("b_mp4_player_fragmented_next_backward:%p %p:(%u) %u:%u", (void *)player, (void *)track, track->info.trackheader.track_ID, (unsigned)tfra_pos, (unsigned)pos));
        if(tfra_pos <= pos ||
           index_entry == 0 /* always include first index entry */
           ) {
            BDBG_MSG(("b_mp4_player_fragmented_next_backward:%p %p:(%u) next %u->%u", (void *)player, (void *)track, track->info.trackheader.track_ID, track->fragment.state.trick.index_entry, index_entry));
            track->fragment.state.trick.index_entry=index_entry;
            break;
        }
    }
    /* 3. Seek fragment to destination */
    b_mp4_fragment_track_clear(&track->fragment);
    BDBG_MSG(("b_mp4_player_fragmented_next_backward:%p %p:(%u) %u -> %u", (void *)player, (void *)track, track->info.trackheader.track_ID, (unsigned)pos, (unsigned) tfra.moof_offset));
    b_mp4_active_box_set(&track->fragment.root, tfra.moof_offset, (uint64_t)(-1), 0);
    track->fragment.state.current.run_start_time = tfra.time;
    track->fragment.state.next.run_start_time = tfra.time;

    /* 4. Read fragment */
    {
        int rc;
        bmp4_track_fragment_run_sample sample;
        bool end_of_file;

        rc = b_mp4_player_track_fragmented_next(player, track, &sample, &end_of_file);
        if(rc<0) {
            return NULL;
        }
        BDBG_MSG(("b_mp4_player_fragmented_next_forward: %p time:%u track_ID:%u flags:%#x", (void *)track,  (unsigned)sample.time,  (unsigned)track->info.trackheader.track_ID, (unsigned)sample.flags));
        track->next_sample.endofstream = false;
        track->next_sample.time = sample.time;
        track->next_sample.offset = sample.offset;
        track->next_sample.len = sample.size;
        track->next_sample.syncsample = true;
        player->status.position=sample.time;
        b_mp4_player_track_fragmented_advance(track);
    }
    return track;
}

static b_mp4_track *
b_mp4_player_fragmented_next(bmp4_player_t player)
{
    b_mp4_track *track;

    if(player->status.direction==0) {
        track = b_mp4_player_fragmented_next_play(player);
    } else if(player->status.direction>0) {
        track = b_mp4_player_fragmented_next_forward(player);
    } else {
        track = b_mp4_player_fragmented_next_backward(player);
    }
    return track;
}

static int
b_mp4_player_fragmented_set_direction(bmp4_player_t player, bmedia_player_step direction, bmedia_time_scale time_scale)
{
    if(player->fragment.index.valid) {
        b_mp4_track *video = b_mp4_player_get_video_track(player);
        if(video) {
            return 0;
        }
    }
    if(direction!=0 || time_scale!=BMEDIA_TIME_SCALE_BASE) {
        BDBG_WRN(("bmp4_player_set_direction: %#lx trick play not supported for fragmented files", (unsigned long)player));
        return BERR_TRACE(-1);
    }
    return 0;
}


static void
b_mp4_sample_finish_entry(bmedia_player_entry *entry)
{
    if(entry->atom) {
        entry->content = bmedia_player_entry_content_payload;
        entry->type = bmedia_player_entry_type_atom;
        entry->length = batom_len(entry->atom);
    } else {
        entry->type = bmedia_player_entry_type_file;
        entry->length = 0;
    }
    return;
}

static void
b_mp4_player_track_clear_accum(b_mp4_track *track)
{
    batom_accum_clear(track->track_accum);
    if(track->accum_data) {
        BKNI_Free(track->accum_data);
        track->accum_data = NULL;
    }
    return;
}

static void
b_mp4_player_make_eos_packet(bmp4_player_t player, bmedia_player_entry *entry)
{
    b_mp4_track *track;
    batom_accum_t dst = player->accum_dest;

    BDBG_MSG(("b_mp4_player_make_eos_packet: %#lx", (unsigned long)player));
    bmedia_player_init_entry(entry);
    batom_accum_clear(player->accum_dest);
    for(track = BLST_SQ_FIRST(&player->movie.tracks); track!=NULL ; track=BLST_SQ_NEXT(track, next)) {
        const batom_vec *eos_vec;
        batom_t eos_atom;
        batom_t eos_pes_atom;
        batom_vec eos_vecs[2];

        switch(track->info.sample_info.entries[0]->sample_type) {
            case bmp4_sample_type_avc:
                eos_vec = &bmedia_eos_h264;
                break;
            case bmp4_sample_type_hevc:
                eos_vec = &bmedia_eos_h265;
                break;
            case bmp4_sample_type_mp4v:
                eos_vec = &bmedia_eos_mpeg4;
                break;
            case bmp4_sample_type_mp4a:
            case bmp4_sample_type_samr:
            case bmp4_sample_type_sawb:
            case bmp4_sample_type_sawp:
            case bmp4_sample_type_qt_ima_adpcm:
            case bmp4_sample_type_qt_ima4_adpcm:
            case bmp4_sample_type_als:
            default:
                eos_vec = NULL;
                break;
        }
        if(eos_vec==NULL) {
            continue;
        }
        eos_vecs[0] = *eos_vec;
        eos_vecs[1] = bmedia_eos_stuffing;
        eos_atom = batom_from_vectors(player->factory, eos_vecs, 2, NULL, NULL);
        if(eos_atom==NULL) {
            continue;
        }
        eos_pes_atom = b_mp4_player_make_pes_packet(player, track, eos_atom, false);
        if(eos_pes_atom==NULL) {
            continue;
        }
        batom_accum_add_atom(dst, eos_pes_atom);
        batom_release(eos_pes_atom);
    }
    batom_accum_add_vec(dst, &bmedia_eos_stuffing);
    entry->length = batom_accum_len(dst);
    entry->atom = batom_from_accum(dst, NULL, NULL);
    entry->type = bmedia_player_entry_type_atom;
    entry->content = bmedia_player_entry_content_payload;
    entry->timestamp = player->status.position;
    return;
}

int
bmp4_player_next(bmp4_player_t player, bmedia_player_entry *entry)
{
    b_mp4_track *track;
    bfile_buffer_result result;

    BDBG_ASSERT(entry);
    BDBG_OBJECT_ASSERT(player, bmp4_player_t);
    bmedia_player_init_entry(entry);

    BDBG_MSG_TRACE(("bmp4_player_next:%p continuation_track:%p current_track:%p", (void *)player, (void *)player->continuation_track, (void *)player->current_track));
    if(player->continuation_track) {
        track = player->continuation_track;
        player->continuation_track = NULL;
        entry->timestamp = track->next_sample.time;
        entry->start = track->next_sample.offset;
        entry->embedded = NULL;
        entry->atom = b_mp4_player_continue_sample(player, track);
        if(entry->atom) {
            b_mp4_sample_finish_entry(entry);
            return 0;
        }
    }
    if(player->current_track) {
        /* continue previous transaction */
        track = player->current_track;
        player->current_track = NULL;
    } else if(player->fragmented) {
        track = b_mp4_player_fragmented_next(player);
    } else if(player->status.direction==0) {
        track = b_mp4_player_next_play(player);
    } else if(player->status.direction>0) {
        track = b_mp4_player_next_forward(player);
    } else {
        track = b_mp4_player_next_backward(player);
    }

    if(track) {
        BDBG_MSG(("bmp4_player_next: %#lx scheduling stream:%u" B_MP4_TYPE_FORMAT " %u(%#lx) time %u", (unsigned long)player, track->info.trackheader.track_ID, B_MP4_TYPE_ARG(track->info.sample_info.entries[0]->type), track->info.trackheader.track_ID, (unsigned long)track, (unsigned)track->next_sample.time));
        if(player->status.direction!=0) {
            b_mp4_player_track_clear_accum(track);
        }
        if(track->next_sample.endofstream) {
    		track->next_sample_valid = false;
            if(!player->eof_reached) {
                player->eof_reached = true;
                b_mp4_player_make_eos_packet(player, entry);
            }  else {
                entry->type = bmedia_player_entry_type_end_of_stream;
	            BDBG_MSG(("bmp4_player_next: %#lx EOF reached", (unsigned long)player));
            }
            return 0;
        }
		entry->timestamp = track->next_sample.time;
		entry->start = track->next_sample.offset;
		entry->embedded = NULL;
		if(player->config.atom_ready) {
			/* start asynchronous read */
			entry->atom = bfile_buffer_async_read(player->config.buffer, track->next_sample.offset, track->next_sample.len, &result, b_mp4_player_read_cont, player);
            if(result==bfile_buffer_result_ok && entry->atom) {
                bool partial_data;
			    entry->atom = b_mp4_player_process_sample(player, track, entry->atom, &partial_data);
                if(partial_data) {
		            track->next_sample_valid = false;
				    entry->type = bmedia_player_entry_type_no_data;
                    entry->length = 0;
                    return 0;
                }
            } else {
                if(entry->atom) {
                    batom_release(entry->atom);
                    entry->atom=NULL;
                }
                player->current_track = track;
                switch(result) {
                case bfile_buffer_result_async:
				    entry->type = bmedia_player_entry_type_async;
                    break;
                case bfile_buffer_result_no_data:
				    entry->type = bmedia_player_entry_type_no_data;
                    break;
                default:
			        player->current_track = NULL;
   				    entry->type = bmedia_player_entry_type_error;
                    break;
                }
                return 0; /* asynchronous read */
            }
        } else {
            bfile_buffer_result result;
            entry->atom = b_mp4_player_load_sample(player, track, &result);
            if(entry->atom==NULL) {
                entry->length = 0;
                switch(result) {
                case bfile_buffer_result_no_data:
                    entry->type = bmedia_player_entry_type_no_data;
                    break;
                default:
                    entry->type = bmedia_player_entry_type_error;
                    break;
                }
                return 0;
            }
        }
        b_mp4_sample_finish_entry(entry);
		track->next_sample_valid = false;
		return 0;
	} 
	BDBG_MSG(("bmp4_player_next: %#lx stream error", (unsigned long)player));
	return -1;
}

void 
bmp4_player_tell(bmp4_player_t player, bmedia_player_pos *pos)
{
	BDBG_OBJECT_ASSERT(player, bmp4_player_t);
	*pos = player->status.position;
	return;
}

void 
bmp4_player_get_status(bmp4_player_t player, bmedia_player_status *status)
{
	BDBG_OBJECT_ASSERT(player, bmp4_player_t);
	BDBG_MSG_TRACE(("bmp4_player_get_status:> %#lx", (unsigned long)player));
	*status = player->status;
	BDBG_MSG_TRACE(("bmp4_player_get_status:< %#lx %ld:%ld", (unsigned long)player, (long)player->status.bounds.first, (long)player->status.bounds.last));
	return;
}

int
bmp4_player_set_direction(bmp4_player_t player, bmedia_player_step direction, bmedia_time_scale time_scale)
{
    int rc=0;
	BDBG_OBJECT_ASSERT(player, bmp4_player_t);
	BDBG_MSG_TRACE(("bmp4_player_set_direction:> %#lx %ld", (unsigned long)player, (long)direction));

    player->eof_reached = false;
    if(player->fragmented) {
        rc = b_mp4_player_fragmented_set_direction(player, direction, time_scale);
        if(rc!=0) {
            goto done;
        }
    }
    if(player->status.direction != direction) {
        player->current_track = NULL;
    }
	player->status.direction = direction;
	player->time_scale = time_scale;
done:
	BDBG_MSG_TRACE(("bmp4_player_set_direction:< %#lx %ld", (unsigned long)player, (long)rc));
	return rc;
}

static bmedia_player_pos
b_mp4_player_fragmented_seek_track(bmp4_player_t player, b_mp4_track *track, bmedia_player_pos pos, bool *endofstream, bool last_sync)
{
    unsigned i;
    unsigned index_entry;
    bool tfra_valid=false;
    bmp4_track_fragment_random_access_payload tfra;

    *endofstream = false;

    if(!track->fragment.index.valid) {
        return BMEDIA_PLAYER_INVALID;
    }
    BKNI_Memset(&tfra, 0, sizeof(tfra));
    BDBG_MSG(("b_mp4_player_fragmented_seek_track:%p %p:(%u) -> %u", (void *)player, (void *)track, track->info.trackheader.track_ID, (unsigned)pos));
    for(index_entry=0,i=0;;i++) {
        bmp4_track_fragment_random_access_payload new_tfra;
        bmedia_player_pos tfra_pos;

        if(i>=track->fragment.index.tfra_header.number_of_entry) {
            *endofstream = true;
            break;
        }
        if(!b_mp4_player_read_track_fragment_random_access_payload(track, i, &new_tfra)) {
            return BMEDIA_PLAYER_INVALID;
        }
        tfra_pos = (1000*new_tfra.time) / track->info.mediaheader.timescale;
        BDBG_MSG(("b_mp4_player_fragmented_seek_track:%p %p:(%u) %u:%u", (void *)player, (void *)track, track->info.trackheader.track_ID, (unsigned)tfra_pos, (unsigned)pos));
        if(tfra_pos >= pos) {
            if(last_sync && tfra_pos > pos) {
                if(tfra_valid) {
                    break;
                }
            }
            index_entry = i;
            tfra = new_tfra;
            tfra_valid = true;
            break;
        }
        tfra_valid = true;
        index_entry = i;
        tfra = new_tfra;
    }
    if(!tfra_valid) {
        return BMEDIA_PLAYER_INVALID;
    }
    track->fragment.state.trick.index_entry = index_entry;
    track->fragment.state.trick.end_of_index = false;
    b_mp4_fragment_track_clear(&track->fragment);
    BDBG_MSG(("b_mp4_player_fragmented_seek_track:%p %p:(%u) %u:%u", (void *)player, (void *)track, track->info.trackheader.track_ID, (unsigned)pos, (unsigned) tfra.moof_offset));
    b_mp4_active_box_set(&track->fragment.root, tfra.moof_offset, (uint64_t)(-1), 0);
    track->fragment.state.current.run_start_time = tfra.time;
    track->fragment.state.next.run_start_time = tfra.time;

    return tfra.time;
}

static bmedia_player_pos
b_mp4_player_fragmented_seek_with_index(bmp4_player_t player, bmedia_player_pos pos)
{
    b_mp4_track *video;
    b_mp4_track *track;
    bmedia_player_pos video_pos;
    bool endofstream;

    video = b_mp4_player_get_video_track(player);
    if(video) {
        video_pos = b_mp4_player_fragmented_seek_track(player, video, pos, &endofstream, false);
        if(video_pos==BMEDIA_PLAYER_INVALID) {
            return video_pos;
        }
    } else {
        video_pos = pos;
    }
    for(track = BLST_SQ_FIRST(&player->movie.tracks); track ; track=BLST_SQ_NEXT(track, next)) {
        if(track==video) {
            continue;
        }
        b_mp4_player_fragmented_seek_track(player, track, pos, &endofstream, true);
    }
    return video_pos;
}

static bmedia_player_pos
b_mp4_player_fragmented_seek(bmp4_player_t player, bmedia_player_pos pos)
{
    b_mp4_track *track;
    if(player->fragment.index.valid) {
        return b_mp4_player_fragmented_seek_with_index(player, pos);
    }
    if(pos!=0)  {
        BDBG_WRN(("b_mp4_player_fragmented_seek: %#lx navigation not supported for fragmented files",(unsigned long)player));
        return BMEDIA_PLAYER_INVALID;
    }
    for(track = BLST_SQ_FIRST(&player->movie.tracks); track ; track=BLST_SQ_NEXT(track, next)) {
        if(track->fragment_valid) {
            b_mp4_fragment_track_reset(player, &track->fragment);
        }
    }
    return 0;
}

int
bmp4_player_seek(bmp4_player_t player, bmedia_player_pos pos)
{
    b_mp4_track *video;
    b_mp4_track *track;
    bmedia_player_pos video_pos;
    bool endofstream;

    BDBG_OBJECT_ASSERT(player, bmp4_player_t);
    BDBG_MSG_TRACE(("bmp4_player_seek:> %#lx %u(%u)", (unsigned long)player, (unsigned)pos, (unsigned)player->status.position));

    player->continuation_track = NULL;
    player->eof_reached = false;

    if(player->fragmented) {
        video_pos = b_mp4_player_fragmented_seek(player, pos);
        if(video_pos == BMEDIA_PLAYER_INVALID) {
            goto error;
        }
        goto done;
    }
    player->current_track = NULL;
    if(pos < player->status.bounds.first || pos > player->status.bounds.last) {
        BDBG_WRN(("bmp4_player_seek: %#lx seek out of bounds %u:%u:%u", (unsigned long)player, (unsigned)player->status.bounds.first, (unsigned)pos, (unsigned)player->status.bounds.last));
        return -1;
    }
    video = b_mp4_player_get_video_track(player);
    if(video) {
        bmedia_player_pos cur_pos;
        for(cur_pos=pos;;) {
            video_pos = bmp4_stream_sample_seek(video->stream, cur_pos, &endofstream);
            if(video_pos != BMEDIA_PLAYER_INVALID) {
                BDBG_MSG(("bmp4_player_seek: %#lx seeked video stream:%u to %u(%u) %s", (unsigned long)player, video->info.trackheader.track_ID, (unsigned)video_pos, (unsigned)cur_pos, endofstream?"endofstream":""));
                break;
            }
            BDBG_WRN(("bmp4_player_seek: %#lx can't seek video stream:%u to %u", (unsigned long)player, video->info.trackheader.track_ID, (unsigned)cur_pos));
            if(!endofstream) {
                goto error;
            }
            if(cur_pos==0 || cur_pos+(BMEDIA_PLAYER_SEEK_TRIES*BMEDIA_PLAYER_SEEK_STEP)<pos) {
                BDBG_ERR(("bmp4_player_seek: %#lx can't seek video stream:%u to %u", (unsigned long)player, video->info.trackheader.track_ID, (unsigned)pos));
                goto error;
            } else if(cur_pos >= BMEDIA_PLAYER_SEEK_STEP) {
                cur_pos -= BMEDIA_PLAYER_SEEK_STEP;
            } else {
                cur_pos = 0;
            }
        }
    } else {
        video_pos = pos;
    }
    player->status.position = video_pos;
    player->rew_stack.position = video_pos;
    player->rew_stack.stack_next = 0;

    for(track = BLST_SQ_FIRST(&player->movie.tracks); track ; track=BLST_SQ_NEXT(track, next)) {
        bmedia_player_pos cur_pos = video_pos;

        batom_accum_clear(track->track_accum);
        if(track->stream == NULL) {
            continue;
        }
        track->next_sample_valid = false;
        if(track==video) {
            continue;
        }
        for(;;) {
            bmedia_player_pos track_pos;
            track_pos = bmp4_stream_sample_seek(track->stream, cur_pos, &endofstream);
            if(track_pos != BMEDIA_PLAYER_INVALID) {
                BDBG_MSG(("bmp4_player_seek: %#lx seeked stream:%u to %u(%u:%u) %s", (unsigned long)player, track->info.trackheader.track_ID, (unsigned)track_pos, (unsigned)cur_pos, (unsigned)pos, endofstream?"endofstream":""));
                break;
            }
            BDBG_WRN(("bmp4_player_seek: %#lx can't seek stream:%u to %u %s", (unsigned long)player, track->info.trackheader.track_ID, (unsigned)cur_pos, endofstream?"endofstream":""));
            if(!endofstream) { goto error;}
            if(cur_pos==0 || cur_pos+(BMEDIA_PLAYER_SEEK_TRIES*BMEDIA_PLAYER_SEEK_STEP)<video_pos) {
                BDBG_ERR(("bmp4_player_seek: %#lx can't seek stream:%u to %u", (unsigned long)player, track->info.trackheader.track_ID, (unsigned)video_pos));
                goto error;
            } else if(cur_pos >= BMEDIA_PLAYER_SEEK_STEP) {
                cur_pos -= BMEDIA_PLAYER_SEEK_STEP;
            } else {
                cur_pos = 0;
            }
        }
    }
done:
    BDBG_MSG_TRACE(("bmp4_player_seek:< %#lx %u:%u", (unsigned long)player, (unsigned)pos, (unsigned)video_pos));
    return 0;
error:
    BDBG_MSG(("bmp4_player_seek:< %#lx %u FAILED", (unsigned long)player, (unsigned)pos));
    return -1;
}

static void *
b_mp4_player_create(bfile_io_read_t fd, const bmedia_player_config *config, const bmedia_player_stream *stream)
{
	return bmp4_player_create(fd, config, stream);
}

static void
b_mp4_player_destroy(void *player)
{
	bmp4_player_destroy(player);
}

static int
b_mp4_player_next(void *player, bmedia_player_entry *entry)
{
	return bmp4_player_next(player, entry);
}

static void 
b_mp4_player_tell(void *player, bmedia_player_pos *pos)
{
	bmp4_player_tell(player, pos);
	return;
}

static void 
b_mp4_player_get_status(void *player, bmedia_player_status *status)
{
	bmp4_player_get_status(player, status);
	return;
}

static int
b_mp4_player_set_direction(void *player, bmedia_player_step direction, bmedia_time_scale time_scale, bmedia_player_decoder_mode *mode)
{
    BSTD_UNUSED(mode);
	return bmp4_player_set_direction(player, direction, time_scale);
}

static int  
b_mp4_player_seek(void *player, bmedia_player_pos pos)
{
	return bmp4_player_seek(player, pos);
}

const bmedia_player_methods bmp4_player_methods = {
	b_mp4_player_create,
	b_mp4_player_destroy,
	b_mp4_player_next,
	b_mp4_player_tell,
	b_mp4_player_get_status,
	b_mp4_player_set_direction,
	b_mp4_player_seek
};



