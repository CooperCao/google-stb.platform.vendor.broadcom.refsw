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
 * AVI player library
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bavi_parser.h"
#include "bavi_player_custom.h"
#include "bfile_cache.h"
#include "blst_squeue.h"
#include "bavi_stream.h" /* only for bavi_stream_status */
#include "bmedia_index.h"

BDBG_MODULE(bavi_player_custom);
#define BDBG_MSG_TRACE(x) BDBG_MSG(x)

BDBG_OBJECT_ID(bavi_player_custom_t);

enum b_avi_player_custom_state {
    b_avi_player_custom_state_header=0, 
    /* b_avi_player_custom_state_sync, */ 
    b_avi_player_custom_state_frames
};

#define B_AVI_GET_DWORD(p,off) \
			((bavi_dword)(((uint8_t *)(p))[(off)+0]) | \
			((bavi_dword)(((uint8_t *)(p))[(off)+1])<<8) | \
			((bavi_dword)(((uint8_t *)(p))[(off)+2])<<16) | \
			((bavi_dword)(((uint8_t *)(p))[(off)+3])<<24))

#define B_AVI_KEYFRAME_BIT	0x10

#define B_AVI_GET_STREAM_ID(p)	(1+(10*(unsigned)((p)[0]-'0') + ((p)[1]-'0')))
#define B_AVI_GET_OBJ_SIZE(p)	(B_AVI_GET_DWORD(p,12))
/* 4 is used to account for the stream ID, needed by the AVI parser */
#define B_AVI_GET_CHUNK_OFF(p)	((B_AVI_GET_DWORD(p,8))-4)
/* 4 + 4 is used to account for the stream ID and object length needed by the AVI parser */
#define B_AVI_GET_CHUNK_SIZE(p)	(((B_AVI_GET_OBJ_SIZE(p)+1)&(~1))+4+4)
#define B_AVI_IS_KEYFRAME(p)	((((uint8_t *)p)[4])&B_AVI_KEYFRAME_BIT)

#define B_AVI_OLD_INDEX_ENTRY ( 4 /* dwChunkId */ + 4 /* dwFlags */ + 4 /* dwOffset */ + 4 /* dwSize */)
/* read multipler of sector */
#define B_AVI_OLD_INDEX_CACHE (4*(512)*B_AVI_OLD_INDEX_ENTRY)


#define B_AVI_PLAYER_SCALE(frame) ((bmedia_player_pos)((((uint64_t)frame) * player->mainheader.dwMicroSecPerFrame)/1000))

typedef struct b_avi_player_custom_object_handler {
	bavi_parser_handler handler; /* must be first member */
	bavi_player_custom_t player;
} b_avi_player_custom_object_handler;

typedef struct b_avi_index_entry {
	unsigned pos; /* position in the AVI index */
	unsigned frame_no; /* current frame number */
	size_t	size; /* size of payload */
	bavi_audio_state_memo audio_memo;
} b_avi_index_entry;

typedef struct b_avi_player_custom_stream {
    BLST_SQ_ENTRY(b_avi_player_custom_stream) link; /* field that is used to link streams together */
    bool header_valid;
    bool stream_info_valid;
    bool end_of_stream;
    unsigned stream_no;
    size_t data_len; /* current length of payload */
    bavi_stream_status status;
    bavi_audio_state audio;
    bmedia_index_t index;
    b_avi_index_entry index_entry;
    bmedia_player_pos index_wait_time;
    bfile_cache_t  idx_cache; /* per stream index cache */
    const uint8_t *next_buf; /* pointer to next entry to be played */
} b_avi_player_custom_stream;

struct bavi_player_custom {
	BDBG_OBJECT(bavi_player_custom_t)
	bfile_io_read_t  fd;
	BLST_SQ_HEAD(b_avi_player_custom_streams, b_avi_player_custom_stream) streams;
	bmedia_player_stream stream;
	enum b_avi_player_custom_state player_state;
	size_t  chunk_size;
	bavi_off_t movi_off;
	bavi_off_t index_movi_off; /* either movi_off if index offset is offset into movi object or 0 if offset into the entire file */
	bavi_dword movi_size;
	bfile_segment idx; /* AVI index */
	bfile_cache_t  idx_cache;
	size_t embedded_length;
	unsigned idx_nentries;
	bavi_mainheader mainheader;
	bool mainheader_valid;
	bool encrypted_video;
	bool dmlh_found;
    bool dmf2_indx;
	unsigned stream_no;
	b_avi_player_custom_object_handler hdrl_handler;
	b_avi_player_custom_object_handler strl_handler;
	b_avi_player_custom_object_handler odml_handler;
    b_avi_player_custom_stream *video_stream;
    bmedia_player_config config;
    bmedia_player_status status;
};

static b_avi_player_custom_stream * b_avi_player_custom_get_video_stream(bavi_player_custom_t player);

static void
b_avi_index_entry_clear(b_avi_index_entry *entry)
{
	entry->pos = 0;
	entry->frame_no = 0;
	entry->size = 0;
	bavi_audio_state_clear_memo(&entry->audio_memo);
	return;
}

static bavi_parser_action
b_avi_player_custom_hdrl(bavi_parser_handler *handler, bavi_fourcc fourcc, batom_t object)
{
	bavi_player_custom_t player = ((b_avi_player_custom_object_handler*)handler)->player;
	BDBG_MSG(("b_avi_player_custom_hdrl: %p " BMEDIA_FOURCC_FORMAT "[" BMEDIA_FOURCC_FORMAT "] %u bytes", (void *)player, BMEDIA_FOURCC_ARG(handler->fourcc), BMEDIA_FOURCC_ARG(fourcc), object?(unsigned)batom_len(object):0));
	BDBG_OBJECT_ASSERT(player, bavi_player_custom_t);
	if (fourcc == BMEDIA_FOURCC('a','v','i','h')) {
		player->mainheader_valid = bavi_read_mainheader(&player->mainheader, object);
	}
	if(object) {
		batom_release(object);
	}
	return bavi_parser_action_none;
}

static bavi_parser_action
b_avi_player_custom_strl(bavi_parser_handler *handler, bavi_fourcc fourcc, batom_t object)
{
	batom_cursor c;
	bavi_player_custom_t player = ((b_avi_player_custom_object_handler*)handler)->player;
	BDBG_OBJECT_ASSERT(player, bavi_player_custom_t);
	BDBG_MSG(("b_avi_player_custom_strl: " BMEDIA_FOURCC_FORMAT "[" BMEDIA_FOURCC_FORMAT "] %u bytes", BMEDIA_FOURCC_ARG(handler->fourcc), BMEDIA_FOURCC_ARG(fourcc), object?(unsigned)batom_len(object):0));
	if(fourcc==BAVI_FOURCC_BEGIN) {
		player->stream_no++;
	}
	if(bmedia_player_stream_test(&player->stream, player->stream_no)) {
		b_avi_player_custom_stream *stream;

		if(fourcc==BAVI_FOURCC_BEGIN) {
			stream = BKNI_Malloc(sizeof(*stream));
			if(stream) {
				stream->stream_no = player->stream_no;
				stream->header_valid = false;
				stream->stream_info_valid = false;
                stream->end_of_stream = false;
				stream->index = NULL;
                stream->idx_cache = NULL;
                stream->next_buf = NULL;

				bavi_audio_state_init(&stream->audio);
				b_avi_index_entry_clear(&stream->index_entry);
				BLST_SQ_INSERT_TAIL(&player->streams, stream, link);
			}
		} else {
			stream  = BLST_SQ_LAST(&player->streams);
			if(stream) {
				switch(fourcc) {
				case BMEDIA_FOURCC('s','t','r','h'):
					BDBG_ASSERT(object);
					stream->header_valid = bavi_read_streamheader(&stream->status.header, object);
					if (stream->header_valid) {
						switch(stream->status.header.fccType) {
						case BMEDIA_FOURCC('a','u','d','s'):
							stream->status.stream_type = bavi_stream_type_audio;
							break;
						case BMEDIA_FOURCC('v','i','d','s'):
							stream->status.stream_type = bavi_stream_type_video;
                            if (!player->video_stream) {
                                char strm_no_0, strm_no_1;
                                player->video_stream = stream;
                                strm_no_0 = '0' | ((player->stream_no-1)/10)%10;
                                strm_no_1 = '0' | (player->stream_no-1)%10;
                            }
							break;
						default:
							stream->status.stream_type = bavi_stream_type_unknown;
							break;
 						}
					}
					break;
				case BMEDIA_FOURCC('s','t','r','f'):
					if(!stream->header_valid) {
						break;
					}
					switch(stream->status.stream_type) {
					case bavi_stream_type_audio:
						batom_cursor_from_atom(&c,object);
						stream->stream_info_valid = bmedia_read_waveformatex(&stream->status.stream_info.audio, &c);
						break;
					case bavi_stream_type_video:
						batom_cursor_from_atom(&c,object);
						stream->stream_info_valid = bmedia_read_bitmapinfo(&stream->status.stream_info.video, &c);
						break;
					default:
						break;
					}
					break;
				case BMEDIA_FOURCC('s','t','r','d'):
					player->encrypted_video = true;
					break;
				case BMEDIA_FOURCC('i','n','d','x'):
					player->dmf2_indx=true;
					break;
				case BAVI_FOURCC_END:
					break;

				default:
					break;
				}
			}
		}
	}

	if(object) {
		batom_release(object);
	}
	return bavi_parser_action_none;
}

static bavi_parser_action
b_avi_player_custom_odml(bavi_parser_handler *handler, bavi_fourcc fourcc, batom_t object)
{
	bavi_player_custom_t player = ((b_avi_player_custom_object_handler*)handler)->player;
	bavi_dmlh dmlh;

	BDBG_MSG(("b_avi_player_custom_odml: %#lx " BMEDIA_FOURCC_FORMAT "[" BMEDIA_FOURCC_FORMAT "] %u bytes", (unsigned long)player, BMEDIA_FOURCC_ARG(handler->fourcc), BMEDIA_FOURCC_ARG(fourcc), object?(unsigned)batom_len(object):0));
	BDBG_OBJECT_ASSERT(player, bavi_player_custom_t);
	switch(fourcc) {
	case BAVI_FOURCC_BEGIN:
		break;
	case BMEDIA_FOURCC('d','m','l','h'):
		if(!bavi_read_dmlh(&dmlh, object)) {
			BDBG_WRN(("b_avi_player_custom_odml: %#lx error in the odml header", (unsigned long)player));
			goto done;
		}
		player->dmlh_found = true;
		if(player->mainheader_valid) {
			player->status.bounds.last = B_AVI_PLAYER_SCALE(dmlh.dwTotalFrames);
		}
		break;
	default:
		break;
	}
done:
	if(object) {
		batom_release(object);
	}
	return bavi_parser_action_none;
}


static bavi_parser_action
b_avi_player_custom_object_begin(void *cntx, bavi_fourcc fourcc, bavi_dword size, bavi_off_t offset)
{
	bavi_player_custom_t player = cntx;

	BDBG_MSG(("b_avi_player_custom_object_begin: %#lx " BMEDIA_FOURCC_FORMAT " %u bytes at %#lx", (unsigned long)cntx, BMEDIA_FOURCC_ARG(fourcc), size, (unsigned long)offset));
	BDBG_OBJECT_ASSERT(player, bavi_player_custom_t);
	switch(fourcc) {
	case BMEDIA_FOURCC('m','o','v','i'):
        player->movi_off = offset;
        player->movi_size = size;
		break;
	case BMEDIA_FOURCC('i','d','x','1'):
		size = size - (size%B_AVI_OLD_INDEX_ENTRY);
		if(size>0) {
			bfile_segment_set(&player->idx, offset, size);
		}
		break;
	default:
		return bavi_parser_action_none;
	}
	return bavi_parser_action_return;
}

static void
b_avi_player_custom_atom_free(batom_t atom, void *user)
{
	void *ptr = *(void **)user;
	BDBG_MSG(("free_avi %#lx", (unsigned long)atom));
	BSTD_UNUSED(atom);
	BKNI_Free(ptr);
	return;
}

static const batom_user b_avi_player_custom_atom = {
	b_avi_player_custom_atom_free,
	sizeof(void **)
};

static bmedia_player_pos
b_avi_player_custom_stream_time(b_avi_player_custom_stream *stream)
{
	bmedia_player_pos time;

	if(stream->status.stream_type == bavi_stream_type_video) {
		time = (uint32_t)(((stream->status.header.dwStart+stream->index_entry.frame_no) * ((uint64_t)stream->status.header.dwScale*1000)/stream->status.header.dwRate));
	} else {
		time = bavi_audio_state_get_timestamp(&stream->audio, stream->index_entry.frame_no, stream->index_entry.size);
	}
	return time;
}


static int b_avi_player_custom_scan_index(bavi_player_custom_t player, unsigned frames_to_probe)
{
    unsigned i;
    b_avi_player_custom_stream *stream;

    for(i=0;i<frames_to_probe;) {
	    const uint8_t *buf;
        unsigned stream_id;

        buf = bfile_cache_next(player->idx_cache);
        if(!buf) {
            if(bfile_cache_is_file_error(player->idx_cache)) {
                return -1;
            }
            break;
        }
        stream_id = B_AVI_GET_STREAM_ID(buf);
	    for(stream=BLST_SQ_FIRST(&player->streams);stream!=NULL;stream=BLST_SQ_NEXT(stream,link)) {
            if(stream->stream_no == stream_id) {
                stream->index_entry.frame_no++;
                i++;
                break;
            }
        }
    }
    return 0;
}

static int b_avi_player_custom_test_index(bavi_player_custom_t player)
{
    unsigned i;
    unsigned frames_to_probe = 50;
    const unsigned min_frames = 5; /* factor of interleave is at least 1:10 */
    unsigned probed_frames;
    b_avi_player_custom_stream *stream;
    int rc;
    bmedia_player_pos video_duration;


    /* we test index that it's */
    /* 1. There are more than one stream */
    /* 2. have not-interleaved audio and video streams */
    /* 2. number of video entries account for 99% percent of stream length */
    if(player->idx_nentries<frames_to_probe) {
        BDBG_MSG(("%s: %p not enough index_entries %u", "b_avi_player_custom_test_index", (void *)player, player->idx_nentries));
        return -1;
    }
    if(player->video_stream==NULL) {
        BDBG_MSG(("%s: %p no video stream", "b_avi_player_custom_test_index", (void *)player));
        return -1;
    }

    for(i=0,stream=BLST_SQ_FIRST(&player->streams);stream!=NULL;stream=BLST_SQ_NEXT(stream,link)) {
        i++;
    }
    frames_to_probe *= i;
    if(i<2) {
        BDBG_MSG(("%s: %p not enough streams %u", "b_avi_player_custom_test_index", (void *)player, i));
        return -1;
    }
    for(stream=BLST_SQ_FIRST(&player->streams);stream!=NULL;stream=BLST_SQ_NEXT(stream,link)) {
        stream->index_entry.frame_no = 0;
    }
    rc = b_avi_player_custom_scan_index(player, frames_to_probe);
    if(rc<0) {
        return rc;
    }

    for(probed_frames=0,i=0,stream=BLST_SQ_FIRST(&player->streams);stream!=NULL;stream=BLST_SQ_NEXT(stream,link)) {
        probed_frames += stream->index_entry.frame_no;
        if(stream->index_entry.frame_no < min_frames) {
            i++;
        }
    }
    BDBG_MSG(("%s: %p probed_frames:%u frames_to_probe:%u factor:%u", "b_avi_player_custom_test_index", (void *)player, probed_frames, frames_to_probe, i));
    if(probed_frames<frames_to_probe) {
        BDBG_MSG(("%s: %p no enough frames", "b_avi_player_custom_test_index", (void *)player));
        return -1;
    }
    if(i==0) {
        BDBG_MSG(("%s: %#lx frames appears to be interleaved", "b_avi_player_custom_test_index", (unsigned long)player));
        return -1;
    }
    /* seek at the end of stream */
    rc = bfile_cache_seek(player->idx_cache, B_AVI_OLD_INDEX_ENTRY*(player->idx_nentries-frames_to_probe));
    if(rc<0) {
        return -1;
    }
    rc = b_avi_player_custom_scan_index(player, frames_to_probe);
    if(rc<0) {
        return rc;
    }
    for(i=0,stream=BLST_SQ_FIRST(&player->streams);stream!=NULL;stream=BLST_SQ_NEXT(stream,link)) {
        if(stream->index_entry.frame_no < min_frames) {
            i++;
        }
    }
    if(i!=0) {
        BDBG_MSG(("%s: %p there is no data for all streams", "b_avi_player_custom_test_index", (void *)player));
        return -1;
    }

    for(stream=BLST_SQ_FIRST(&player->streams);stream!=NULL;stream=BLST_SQ_NEXT(stream,link)) {
        stream->index_entry.frame_no = 0;
    }

    rc = bfile_cache_seek(player->idx_cache, 0);
    if(rc<0) {
        return -1;
    }
    stream = player->video_stream;
    stream->index_entry.frame_no = 0;

    for(;;) {
        const uint8_t *buf;

        buf = bfile_cache_next(player->idx_cache);
        if(!buf) {
            if(bfile_cache_is_file_error(player->idx_cache)) {
                return -1;
            }
            break;
        }
        if(stream->stream_no==B_AVI_GET_STREAM_ID(buf)) {
            stream->index_entry.frame_no++;
        }
    }
    video_duration = b_avi_player_custom_stream_time(stream);
    BDBG_MSG(("%s: %p video_duration: %u stream_duration: %u", "b_avi_player_custom_test_index", (void *)player, (unsigned)video_duration, (unsigned)player->status.bounds.last));
    stream->index_entry.frame_no = 0;
    rc = bfile_cache_seek(player->idx_cache, 0);
    if(rc<0) {
        return -1;
    }
    return 0;
}

static int
b_avi_player_custom_open_index(bavi_player_custom_t player)
{
	batom_factory_t factory;
	batom_pipe_t pipe_avi;
	batom_t atom;
	bfile_io_read_t fd;
	bavi_parser_cfg cfg;
	bavi_parser_t  avi;
	enum {b_avi_player_custom_open_state_movi, b_avi_player_custom_open_state_idx} state;
	int rc=-1;
	b_avi_player_custom_stream *stream;
    size_t parsed_size;
    off_t idx_off;
    off_t seek_result;


	BDBG_OBJECT_ASSERT(player, bavi_player_custom_t);
    fd = player->fd;
	state = b_avi_player_custom_open_state_movi;
	player->movi_off = -1;
	player->movi_size = 0;
	bfile_segment_clear(&player->idx);
	player->mainheader_valid = false;
	player->stream_no = 0;
    player->idx_nentries = 0;

	factory = batom_factory_create(bkni_alloc, 64);
	if(!factory) {
		goto err_factory;
	}

	bavi_parser_default_cfg(&cfg);
	cfg.user_cntx = player;
	cfg.object_begin = b_avi_player_custom_object_begin;
	avi = bavi_parser_create(factory, &cfg);
	if(!avi) {
		goto err_avi;
	}
	player->hdrl_handler.player = player;
	player->strl_handler.player = player;
	player->odml_handler.player = player;
	bavi_parser_install_handler(avi, &player->hdrl_handler.handler, BMEDIA_FOURCC('h','d','r','l'), b_avi_player_custom_hdrl);
	bavi_parser_install_handler(avi, &player->strl_handler.handler, BMEDIA_FOURCC('s','t','r','l'), b_avi_player_custom_strl);
	bavi_parser_install_handler(avi, &player->odml_handler.handler, BMEDIA_FOURCC('o','d','m','l'), b_avi_player_custom_odml);
	pipe_avi = batom_pipe_create(factory);
	if (!pipe_avi) {
		goto err_pipe;
	}
	fd->seek(fd, 0, SEEK_SET);
	for(parsed_size=0;parsed_size<=player->config.max_data_parsed;) {
		void *buf;
		ssize_t read_result;
		size_t avi_result;
		const size_t len = player->config.prefered_read_size;

		buf = BKNI_Malloc(len);
		if (!buf) {
			goto err_alloc;
		}
        BDBG_MSG(("b_avi_player_custom_open_index: %p reading %u bytes (%u)", (void *)player, (unsigned)len, (unsigned)parsed_size));
		parsed_size += len;
		read_result =  fd->read(fd, buf, len);
		if(read_result<0) {
			BKNI_Free(buf);
			goto err_read;
		}
		atom = batom_from_range(factory, buf, read_result, &b_avi_player_custom_atom, &buf);
		if(!atom) {
			BKNI_Free(buf);
			goto err_atom;
		}
		batom_pipe_push(pipe_avi, atom);
		avi_result = bavi_parser_feed(avi, pipe_avi);
		if(avi_result==0) {
			goto err_parser;
		}

		if(state == b_avi_player_custom_open_state_movi && player->movi_off>=0) {
			/* located movi object */
			state = b_avi_player_custom_open_state_idx;
			batom_pipe_flush(pipe_avi);
		    idx_off =  player->movi_off + player->movi_size;
			BDBG_MSG(("b_avi_player_custom_open_index: seek to " BDBG_UINT64_FMT "movi (" BDBG_UINT64_FMT ")", BDBG_UINT64_ARG(idx_off), BDBG_UINT64_ARG(player->movi_off)));

			seek_result = fd->seek(fd, idx_off, SEEK_SET);
			if(seek_result != idx_off) {
				goto err_seek;
			}			
			bavi_parser_seek(avi, idx_off); /* skip movi object */
		} else if(state == b_avi_player_custom_open_state_idx && bfile_segment_test(&player->idx)) {
            idx_off =  player->idx.start + player->idx.len;
            seek_result = fd->seek(fd, 0, SEEK_END);			
            if(seek_result < idx_off) {
                goto err_seek;
            }				
            player->idx_nentries = player->idx.len / B_AVI_OLD_INDEX_ENTRY;

			if(player->mainheader_valid && player->idx_nentries>0) {
				rc = 0; /* have found location of the index */
			} 
            break;
		}
	}

	if(rc!=0) {
		BDBG_WRN(("b_avi_player_custom_open_index: %#lx no valid streams found", (unsigned long)player));
        goto err_file;
	}
    if(player->idx_nentries<=0) {
        rc = -1;
        BDBG_MSG(("b_avi_player_custom_open_index: %#lx no index", (unsigned long)player));
        goto err_file;
    }
    player->idx_cache = bfile_cache_create(fd, &player->idx, B_AVI_OLD_INDEX_CACHE, B_AVI_OLD_INDEX_ENTRY);

    if(!player->idx_cache) {
        goto err_cache;
    }
    rc = b_avi_player_custom_test_index(player);
    if(rc!=0) {
        goto err_file_type;
    }
   
	if(player->encrypted_video) {
        BDBG_MSG(("b_avi_player_custom_open_index: %#lx encrypted file", (unsigned long)player));
        /* these types of files aren't supported here */
        rc = -1;
        goto err_dmf2_indx;
	}
	if(player->status.bounds.last==0) {
		player->status.bounds.last = B_AVI_PLAYER_SCALE(player->mainheader.dwTotalFrames);
	}
	BDBG_MSG(("b_avi_player_custom_open_index: %#lx bounds:%u..%u", (unsigned long)player, (unsigned)player->status.bounds.first, (unsigned)player->status.bounds.last));

	for(stream=BLST_SQ_FIRST(&player->streams);stream!=NULL;) {
                unsigned i;
		b_avi_player_custom_stream *next_stream = BLST_SQ_NEXT(stream, link);

        if(stream->header_valid && stream->stream_info_valid && stream->status.header.dwRate) {
            bmedia_player_pos duration = player->status.bounds.last;

            BDBG_MSG(("b_avi_player_custom_open_index: %p stream_no:%u duration %u", (void *)player, (unsigned)stream->stream_no, (unsigned)duration));
            stream->index = bmedia_index_create(duration, sizeof(b_avi_index_entry), 100);
        }
        if(!stream->index) {
            rc = -1;
            goto err_stream;
        }
        BDBG_WRN(("Stream %u %p", (unsigned)stream->stream_no, (void*)stream->index));
        if(stream->status.stream_type == bavi_stream_type_audio) {
            bavi_audio_state_set_header(&stream->audio, &stream->status.header, &stream->status.stream_info.audio);
        }
        stream->index_wait_time = bmedia_index_next(stream->index);
        stream->idx_cache = bfile_cache_create(fd, &player->idx, B_AVI_OLD_INDEX_CACHE/2, B_AVI_OLD_INDEX_ENTRY);
        if(!stream->idx_cache) {
            goto err_stream;
        }
        for(i=0;;i++) {
            const uint8_t *buf;
            buf = bfile_cache_next(player->idx_cache);
            if(!buf) {
                BDBG_MSG(("b_avi_player_custom_open_index: %p  can't find index entry for stream_no:%u", (void *)player, (unsigned)stream->stream_no));
                rc = -1;
                goto err_stream;
            }
            if(stream->stream_no == B_AVI_GET_STREAM_ID(buf)) {
                BDBG_MSG(("b_avi_player_custom_open_index: %p stream_no:%u first_index:%u", (void *)player, (unsigned)stream->stream_no, i));
                stream->next_buf = buf;
                break;
            }
        }
        stream = next_stream;
    }
    bfile_cache_seek(player->idx_cache, 0);
    goto done;

err_stream:
err_dmf2_indx:
err_file_type:
	bfile_cache_destroy(player->idx_cache);
done:
err_cache:
err_file:
err_seek:
err_parser:
err_atom:
err_read:
err_alloc:
	batom_pipe_flush(pipe_avi);
	batom_pipe_destroy(pipe_avi);
err_pipe:
	bavi_parser_remove_handler(avi, &player->odml_handler.handler);
	bavi_parser_remove_handler(avi, &player->strl_handler.handler);
	bavi_parser_remove_handler(avi, &player->hdrl_handler.handler);
	bavi_parser_destroy(avi);
err_avi:
	batom_factory_destroy(factory);
err_factory:
	return rc;
}


static void
b_avi_player_custom_free_streams(bavi_player_custom_t player)
{
	b_avi_player_custom_stream *stream;
	while(NULL!=(stream=BLST_SQ_FIRST(&player->streams))) {
		BLST_SQ_REMOVE_HEAD(&player->streams, link);
		if(stream->index) {
			bmedia_index_destroy(stream->index);
		}
	    if(stream->idx_cache) {
            bfile_cache_destroy(stream->idx_cache);
        }
		BKNI_Free(stream);
	}
	return;
}


bavi_player_custom_t
bavi_player_custom_create(bfile_io_read_t fd, const bmedia_player_config *config, const bmedia_player_stream *stream)
{
	bavi_player_custom_t player;
	int rc;
	char strm_no[2];
	const uint8_t *buf;


	BDBG_ASSERT(fd);
	BDBG_ASSERT(config);
	BDBG_ASSERT(stream && stream->format == bstream_mpeg_type_avi);

	player = BKNI_Malloc(sizeof(*player));
	if (!player) {
		BDBG_ERR(("can't allocated %u bytes", (unsigned)sizeof(*player)));
		goto err_alloc;
	}
	BDBG_OBJECT_INIT(player, bavi_player_custom_t);
	player->fd = fd;
	player->config = *config;
	player->stream = *stream;
	BLST_SQ_INIT(&player->streams);
	strm_no[0] = '0' | (((stream->master-1)/10)%10);
	strm_no[1] = '0' | ((stream->master-1)%10);
	player->encrypted_video = false;
	player->dmf2_indx=false;
	player->video_stream = NULL;
	player->player_state = b_avi_player_custom_state_header;
    bmedia_player_init_status(&player->status);

	if(b_avi_player_custom_open_index(player)<0) {
		BDBG_MSG(("no index"));
		goto err_index;
	}
	if(!BLST_SQ_FIRST(&player->streams)) {
		BDBG_MSG(("no streams"));
		goto err_index;
	}

	player->status.position = 0;

	player->chunk_size = 60 * 1024;
	player->embedded_length = 0;
	player->dmlh_found = false;

	buf = bfile_cache_next(player->idx_cache);
	if(!buf) {
		goto err_index_off;
	}

    if(B_AVI_GET_CHUNK_OFF(buf)+4/*LIST*/+4/*Length*/+4/*movi*/ < player->movi_off) {
        player->index_movi_off = player->movi_off;
    } else {
        BDBG_MSG(("bavi_player_custom_create: %p index uses absolute offsets %u:%u", (void *)player, (unsigned)B_AVI_GET_CHUNK_OFF(buf)+4+4+4, (unsigned)player->movi_off));
        player->index_movi_off = 4;
    }
	rc = bfile_cache_seek(player->idx_cache, 0);
	if(rc<0) {
		goto err_cache;
	}

	return player;

err_index_off:
err_cache:
	bfile_cache_destroy(player->idx_cache);
err_index:
	b_avi_player_custom_free_streams(player);
	BKNI_Free(player);
err_alloc:
	return NULL;
}

void
bavi_player_custom_destroy(bavi_player_custom_t player)
{
	BDBG_OBJECT_ASSERT(player, bavi_player_custom_t);

	bfile_cache_destroy(player->idx_cache);
	b_avi_player_custom_free_streams(player);
	BDBG_OBJECT_DESTROY(player, bavi_player_custom_t);
	BKNI_Free(player);
	return;
}





static b_avi_player_custom_stream *
b_avi_player_custom_get_video_stream(bavi_player_custom_t player)
{
	b_avi_player_custom_stream *stream = player->video_stream;
    if (!stream) {
	    for(stream=BLST_SQ_FIRST(&player->streams);stream;stream=BLST_SQ_NEXT(stream, link)) {
		    if(stream->status.stream_type == bavi_stream_type_video) {
			    break;
		    }
	    }
    }
	return stream;
}


static b_avi_player_custom_stream *
bavi_player_custom_select_stream(bavi_player_custom_t player)
{
    b_avi_player_custom_stream *min_stream=NULL;
    bmedia_player_pos min_pos=0;
    b_avi_player_custom_stream *stream;

    for(stream=BLST_SQ_FIRST(&player->streams);stream!=NULL;stream=BLST_SQ_NEXT(stream,link)) {
        bmedia_player_pos pos;
        if(stream->end_of_stream) {
            continue;
        }
        pos = b_avi_player_custom_stream_time(stream);
        if(min_stream==NULL || pos<min_pos) {
            BDBG_MSG(("bavi_player_custom_select_stream: %p choosing stream %p(%u) instead %p(%u)", (void *)player, (void *)stream, (unsigned)pos, (void *)min_stream, (unsigned)min_pos));
            min_pos = pos;
            min_stream = stream;
        }
    }
    return min_stream;
}

static void
b_avi_player_custom_update_audio_stream(bavi_player_custom_t player, b_avi_player_custom_stream *stream, const uint8_t *buf)
{
    BSTD_UNUSED(player);
    if(stream->status.stream_type == bavi_stream_type_audio) {
        size_t obj_size = B_AVI_GET_OBJ_SIZE(buf);
        bavi_audio_state_update(&stream->audio, obj_size);
    }
}

static void
b_avi_player_custom_update_stream(bavi_player_custom_t player, b_avi_player_custom_stream *stream, const uint8_t *buf)
{
	size_t obj_size = B_AVI_GET_OBJ_SIZE(buf);
    BSTD_UNUSED(player);
	stream->index_entry.frame_no++;
	stream->index_entry.size += obj_size;
	return;
}

static int
bavi_player_custom_advance_stream_next(bavi_player_custom_t player, b_avi_player_custom_stream *stream)
{
    BSTD_UNUSED(player);
    for(;;) {
        const uint8_t *buf;

        buf = bfile_cache_next(stream->idx_cache);
        if(!buf) {
            if(bfile_cache_is_file_error(stream->idx_cache)) {
                return -1;
            }
            stream->end_of_stream = true;
            break;
        }
        if(stream->stream_no==B_AVI_GET_STREAM_ID(buf)) {
            stream->next_buf = buf;
            break;
        }
    }
    return 0;
}

static int 
bavi_player_custom_advance_stream(bavi_player_custom_t player, b_avi_player_custom_stream *stream)
{
    if(stream->end_of_stream) {
        return -1;
    }
    b_avi_player_custom_update_audio_stream(player, stream, stream->next_buf);
    b_avi_player_custom_update_stream(player, stream, stream->next_buf);
    return bavi_player_custom_advance_stream_next(player, stream);
}

static int 
bavi_player_custom_next_frame_play(bavi_player_custom_t player, bmedia_player_entry *entry)
{
    b_avi_player_custom_stream *stream;
	bavi_off_t block_begin;

    stream = bavi_player_custom_select_stream(player);
    if(stream==NULL) {
        BDBG_MSG(("bavi_player_custom_next_frame_play: %#lx end of stream", (unsigned long)player));
	    entry->type = bmedia_player_entry_type_end_of_stream;
        return 0;
    }
    block_begin = B_AVI_GET_CHUNK_OFF(stream->next_buf);
	entry->start = block_begin+player->index_movi_off;
    entry->length = B_AVI_GET_CHUNK_SIZE(stream->next_buf);
    entry->embedded = NULL;
    entry->content =  bmedia_player_entry_content_payload;
    return bavi_player_custom_advance_stream(player, stream);
}

int
bavi_player_custom_next(bavi_player_custom_t player, bmedia_player_entry *entry)
{
    int rc;
	BDBG_OBJECT_ASSERT(player, bavi_player_custom_t);
	BDBG_MSG_TRACE(("bavi_player_custom_next:> %#lx %u %d", (unsigned long)player, (unsigned)player->status.position, (int)player->status.direction));
    bmedia_player_init_entry(entry);

	entry->type = bmedia_player_entry_type_file;
    switch(player->player_state) {
    case b_avi_player_custom_state_frames:
        rc = bavi_player_custom_next_frame_play(player, entry);
        break;
    case b_avi_player_custom_state_header:
		entry->start =  0 ;
		entry->length = player->movi_off;
		entry->embedded = NULL;
        entry->content =  bmedia_player_entry_content_header;
		player->player_state = b_avi_player_custom_state_frames;
        rc = 0;
        break;
    default:
        rc = -1;
        break;
    }
	BDBG_MSG(("bavi_player_custom_next: %#lx %u (%lu:%u) %s", (unsigned long)player, (unsigned)player->status.position, (unsigned long)entry->start, (unsigned)entry->length, entry->embedded?"embedded":""));
	BDBG_MSG_TRACE(("bavi_player_custom_next:< %#lx %u (%lu:%u:%lu) %s", (unsigned long)player, (unsigned)player->status.position, (unsigned long)entry->start, (unsigned)entry->length, (unsigned long)entry->timestamp, entry->embedded?"embedded":""));
	return 0;
}

void
bavi_player_custom_tell(bavi_player_custom_t player, bmedia_player_pos *pos)
{
	b_avi_player_custom_stream *stream;
	BDBG_OBJECT_ASSERT(player, bavi_player_custom_t);

	stream = b_avi_player_custom_get_video_stream(player);
	if(stream) {
	    *pos = b_avi_player_custom_stream_time(stream);
    } else {
		*pos = BMEDIA_PLAYER_INVALID;
	}
	return;
}

void
bavi_player_custom_get_status(bavi_player_custom_t player, bmedia_player_status *status)
{
	BDBG_OBJECT_ASSERT(player, bavi_player_custom_t);
	BDBG_MSG_TRACE(("bavi_player_custom_get_status:> %#lx", (unsigned long)player));
    *status = player->status;
    bavi_player_custom_tell(player, &status->position);
	BDBG_MSG_TRACE(("bavi_player_custom_get_status:< %#lx %ld:%ld %ld", (unsigned long)player, (long)player->status.bounds.first, (long)player->status.bounds.last, (long)player->status.direction));
	return;
}

void
bavi_player_custom_set_direction(bavi_player_custom_t player, bmedia_player_step direction)
{
	BDBG_OBJECT_ASSERT(player, bavi_player_custom_t);
	BDBG_MSG_TRACE(("bavi_player_custom_set_direction:> %#lx %ld", (unsigned long)player, (long)direction));
	player->status.direction = direction;
	BDBG_MSG_TRACE(("bavi_player_custom_set_direction:< %#lx %ld", (unsigned long)player, (long)player->status.direction));
	return;
}





int
bavi_player_custom_seek(bavi_player_custom_t player, bmedia_player_pos pos)
{
    bmedia_player_pos index_pos = pos;
    bmedia_player_pos video_time = pos;
    b_avi_player_custom_stream *stream;
    int rc = 0;

    BSTD_UNUSED(index_pos);
    BSTD_UNUSED(video_time);

    BDBG_OBJECT_ASSERT(player, bavi_player_custom_t);
    BDBG_MSG_TRACE(("bavi_player_custom_seek:> %p %ld", (void *)player, (long)pos));
    if (pos >= player->status.bounds.last || pos < player->status.bounds.first) {
        BDBG_WRN(("bavi_player_custom_seek:< %p out of bounds %u<%u>%u %ld", (void *)player, (unsigned)player->status.bounds.first, (unsigned)pos, (unsigned)player->status.bounds.last, (long)-1));
        pos = player->status.bounds.first;
    }
    /* XXX always seek to beginning */
    player->player_state = b_avi_player_custom_state_header;
    for(stream=BLST_SQ_FIRST(&player->streams);stream;stream=BLST_SQ_NEXT(stream, link)) {
        stream->end_of_stream = false;
        rc = bfile_cache_seek(stream->idx_cache, 0);
        b_avi_index_entry_clear(&stream->index_entry);
        if(rc!=0) {
            break;
        }
        rc = bavi_player_custom_advance_stream_next(player, stream);
        if(rc!=0) {
            break;
        }
    }
    BDBG_MSG_TRACE(("bavi_player_custom_seek:< %p %u:%u:%u", (void *)player, (unsigned)video_time,(unsigned)pos, (unsigned)index_pos));
    return rc;
}

static void *
b_avi_player_custom_create(bfile_io_read_t fd, const bmedia_player_config *config, const bmedia_player_stream *stream)
{
	return bavi_player_custom_create(fd, config, stream);
}

static void
b_avi_player_custom_destroy(void *player)
{
	bavi_player_custom_destroy(player);
}

static int
b_avi_player_custom_next(void *player, bmedia_player_entry *entry)
{
	return bavi_player_custom_next(player, entry);
}

static void
b_avi_player_custom_tell(void *player, bmedia_player_pos *pos)
{
	bavi_player_custom_tell(player, pos);
	return;
}

static void
b_avi_player_custom_get_status(void *player, bmedia_player_status *status)
{
	bavi_player_custom_get_status(player, status);
	return;
}

static int
b_avi_player_custom_set_direction(void *player, bmedia_player_step direction, bmedia_time_scale time_scale, bmedia_player_decoder_mode *mode)
{
	BSTD_UNUSED(time_scale);
    BSTD_UNUSED(mode);
	bavi_player_custom_set_direction(player, direction);
	return 0;
}

static int
b_avi_player_custom_seek(void *player, bmedia_player_pos pos)
{
	return bavi_player_custom_seek(player, pos);
}

const bmedia_player_methods bavi_player_custom_methods = {
	b_avi_player_custom_create,
	b_avi_player_custom_destroy,
	b_avi_player_custom_next,
	b_avi_player_custom_tell,
	b_avi_player_custom_get_status,
	b_avi_player_custom_set_direction,
	b_avi_player_custom_seek
};


