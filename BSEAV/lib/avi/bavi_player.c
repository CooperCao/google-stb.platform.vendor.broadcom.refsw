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
 *
 * Module Description:
 *
 * AVI player library
 *
 *******************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bavi_parser.h"
#include "bavi_player.h"
#include "bfile_cache.h"
#include "blst_squeue.h"
#include "bavi_stream.h" /* only for bavi_stream_status */
#include "bmedia_index.h"

BDBG_MODULE(bavi_player);
#define BDBG_MSG_TRACE(x) BDBG_MSG(x)

BDBG_OBJECT_ID(bavi_player_t);


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


#define B_AVI_GET_STD_OBJ_SIZE(p)       (B_AVI_GET_DWORD(p,4)&(~(1<<31)))
/* 8 is used to account for the stream ID, needed by the AVI parser */
#define B_AVI_GET_STD_CHUNK_OFF(p)	(((bavi_off_t)B_AVI_GET_DWORD(p,0))-8)
/* 4 + 4 is used to account for the stream ID and object length needed by the AVI parser */
#define B_AVI_GET_STD_CHUNK_SIZE(p)	(((B_AVI_GET_STD_OBJ_SIZE(p)+1)&((~1)))+4+4)
#define B_AVI_IS_STD_KEYFRAME(p)	(!(B_AVI_GET_DWORD(p,4)>>31))
#define B_AVI_STD_INDEX_OFFSET (4 /* FCC */ + 4 /* Size */ + 2 /* wLongsPerEntry */ + 1 /* bIndexSubType*/ + 1 /* bIndexType*/  + 4 /* dwEntriesInUse */ + 4 /* dwChunkId */ + 8 /* qwBaseOffset */  + 4 /* dwReserved */)
#define B_AVI_STD_INDEX_ENTRY (4 /* dwOffset */ + 4 /* dwSize */)
#define B_AVI_STD_INDEX_CACHE (4*(512)*B_AVI_STD_INDEX_ENTRY)


#define  B_AVI_DRM_HEADER_SIZE 18
#define  B_AVI_IS_DRM_HEADER(p) ((p[2] == 'd')&&(p[3] == 'd'))

#define  B_AVI_IS_SUBTITLE_HEADER(p) ((p[2] == 's')&&(p[3] == 'b'))

#define B_AVI_PLAYER_SCALE(frame) ((bmedia_player_pos)((((uint64_t)frame) * player->mainheader.dwMicroSecPerFrame)/1000))

#define DMF2_SEGMENT_SIZE 128*1024

#define B_AVIX_HEADER_SIZE 24
#define B_AVI_AUDIO_INDEX_THRESHOLD 3000

enum b_avi_player_state {
    b_avi_player_state_header=0, 
    /* b_avi_player_state_sync, */ 
    b_avi_player_state_data, 
    b_avi_player_state_embedded, 
    b_avi_player_state_avix_header, 
    b_avi_player_state_dmf2_segment,
    b_avi_player_state_preframe 
};

typedef struct b_avi_player_object_handler {
	bavi_parser_handler handler; /* must be first member */
	bavi_player_t player;
} b_avi_player_object_handler;

typedef struct b_avi_index_entry {
	unsigned pos; /* position in the AVI index */
	unsigned frame_no; /* current frame number */
	size_t	size; /* size of payload */
	bavi_audio_state_memo audio_memo;
} b_avi_index_entry;

typedef struct b_avi_superindex_entry {
		    uint64_t qwOffset;
		    unsigned dwSize;
		    unsigned dwDuration;
} b_avi_superindex_entry;

typedef struct b_avi_superindexchunk {
	  uint16_t wLongsPerEntry;
	  uint8_t bIndexSubType;
	  uint8_t bIndexType;
	  unsigned nEntriesInUse;
	  unsigned dwChunkId;
	  unsigned dwReserved[3];
	  b_avi_superindex_entry * avi_superindex_entry;
} b_avi_superindexchunk;

typedef struct b_avi_stdindexchunk {
	  uint16_t wLongsPerEntry;
	  uint8_t bIndexSubType;
	  uint8_t bIndexType;
	  unsigned nEntriesInUse;
	  unsigned dwChunkId;
	  uint64_t qwBaseOffset;
	  unsigned dwReserved3;
} b_avi_stdindexchunk;

typedef struct b_avi_dmf2_riff_info {
	  bavi_off_t start;
	  bavi_off_t len;
	  bavi_off_t movi_off;
}b_avi_dmf2_riff_info;

typedef struct b_avi_dmf2_info {
	  bavi_off_t dmf2_start_offset;
	  bfile_segment dmf2_idx[8]; /*Array of index for each audio track in AVI2.0 index*/
	  bfile_cache_t dmf2_idx_cache[8];
	  int32_t dmf2_resync_offset[8];
	  unsigned dmf2_index_acc;
	  b_avi_dmf2_riff_info riff_info[32];
	  uint8_t curr_dmf2_riff_indx;
	  struct {
		    bavi_off_t start, end;
	  } segment;
}b_avi_dmf2_info;

typedef struct b_avi_player_stream {
	  BLST_SQ_ENTRY(b_avi_player_stream) link; /* field that is used to link streams together */
	  bool header_valid;
	  bool stream_info_valid;
	  bool seek_completed;
	  unsigned stream_no;
	  size_t data_len; /* current length of payload */
	  bavi_stream_status status;
	  bmedia_index_t index;
	  bavi_audio_state audio;
	  b_avi_index_entry index_entry;
	  bmedia_player_pos index_wait_time;
	  b_avi_superindexchunk superindex;
	  b_avi_stdindexchunk stdindex;
	  int cur_superindex_entry;
} b_avi_player_stream;

struct bavi_player {
	BDBG_OBJECT(bavi_player_t)
	bfile_io_read_t  fd;
	BLST_SQ_HEAD(b_avi_player_streams, b_avi_player_stream) streams;
	bmedia_player_stream stream;
	bavi_fourcc stream_fourcc;
	enum b_avi_player_state player_state;
	size_t  chunk_size;
	bavi_off_t last_read_data_offset;
	bavi_off_t movi_off;
	bavi_off_t index_movi_off; /* either movi_off if index offset is offset into movi object or 0 if offset into the entire file */
	bavi_dword movi_size;
	bavi_off_t idx1_off;
	bavi_dword idx1_size;
    bavi_off_t first_vframe_off;
    bavi_off_t first_vframe_size;
	bfile_segment idx; /* AVI index */
	bfile_cache_t  idx_cache;
	size_t embedded_length;
	unsigned idx_nentries;
	unsigned pos; /* current position in the index */
	int step;
	bmedia_player_bounds bounds;
	bavi_mainheader mainheader;
	bool mainheader_valid;
	bmedia_player_sync_entry resync;
	bool need_timestamp;
    bool preframe_required;
	unsigned stream_no;
	b_avi_player_object_handler hdrl_handler;
	b_avi_player_object_handler strl_handler;
	b_avi_player_object_handler odml_handler;
    bavi_fourcc vchunk_fourccs[3];
    b_avi_player_stream *video_stream;
	uint8_t embedded_data[BIO_BLOCK_SIZE];
	bool encrypted_video;
	bool eof_reached;
	bool dmlh_found;
	uint8_t zero[1];
	bool dmf2_indx;
	b_avi_dmf2_info dmf2_info;
    bmedia_player_config config;
	unsigned index_error_cnt; 
};

static b_avi_player_stream * b_avi_player_get_video_stream(bavi_player_t player);

static void
b_avi_player_atom_free(batom_t atom, void *user)
{
	void *ptr = *(void **)user;
	BDBG_MSG(("free_avi %#lx", (unsigned long)atom));
	BSTD_UNUSED(atom);
	BKNI_Free(ptr);
	return;
}

static const batom_user b_avi_player_atom = {
	b_avi_player_atom_free,
	sizeof(void **)
};

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
b_avi_player_hdrl(bavi_parser_handler *handler, bavi_fourcc fourcc, batom_t object)
{
	bavi_player_t player = ((b_avi_player_object_handler*)handler)->player;
	BDBG_MSG(("b_avi_player_hdrl: %p " BMEDIA_FOURCC_FORMAT "[" BMEDIA_FOURCC_FORMAT "] %u bytes", (void *)player, BMEDIA_FOURCC_ARG(handler->fourcc), BMEDIA_FOURCC_ARG(fourcc), object?(unsigned)batom_len(object):0));
	BDBG_OBJECT_ASSERT(player, bavi_player_t);
	if (fourcc == BMEDIA_FOURCC('a','v','i','h')) {
		player->mainheader_valid = bavi_read_mainheader(&player->mainheader, object);
	}
	if(object) {
		batom_release(object);
	}
	return bavi_parser_action_none;
}

static bool
b_avi_read_superindex(b_avi_player_stream *stream, batom_t object)
{

	batom_cursor c;
	unsigned cnt;

	batom_cursor_from_atom(&c,object);

	stream->superindex.wLongsPerEntry =  bavi_read_word(&c);
	stream->superindex.bIndexSubType = bavi_read_byte(&c);
	stream->superindex.bIndexType = bavi_read_byte(&c);
	stream->superindex.nEntriesInUse =  bavi_read_dword(&c);

	stream->superindex.dwChunkId = bavi_read_fourcc(&c);
	/*skip over 12 reserved bytes*/
	batom_cursor_skip(&c, sizeof(stream->superindex.dwReserved));

	BDBG_WRN(("wLongsPerEntry : %u, bIndexSubType : %u, bIndexType : %u,  nEntriesInUse : %u, dwChunkId " BMEDIA_FOURCC_FORMAT " ",
		  stream->superindex.wLongsPerEntry, stream->superindex.bIndexSubType, stream->superindex.bIndexType, stream->superindex.nEntriesInUse, BMEDIA_FOURCC_ARG(stream->superindex.dwChunkId)));

	stream->superindex.avi_superindex_entry = BKNI_Malloc(stream->superindex.nEntriesInUse*sizeof(*(stream->superindex.avi_superindex_entry)));

	for(cnt=0; cnt<stream->superindex.nEntriesInUse; cnt++) {
		stream->superindex.avi_superindex_entry[cnt].qwOffset = bavi_read_qword(&c);
		stream->superindex.avi_superindex_entry[cnt].dwSize = bavi_read_dword(&c);
		stream->superindex.avi_superindex_entry[cnt].dwDuration = bavi_read_dword(&c);

		BDBG_MSG(("qwOffset : %#lx, dwSize :%u, dwDuration : %u",(unsigned long)stream->superindex.avi_superindex_entry[cnt].qwOffset, stream->superindex.avi_superindex_entry[cnt].dwSize, stream->superindex.avi_superindex_entry[cnt].dwDuration));
	}

	stream->cur_superindex_entry = 0;

	return !BATOM_IS_EOF(&c);
}

static int
b_avi_read_stdindex(bavi_player_t player, b_avi_player_stream *stream)
{
	bfile_io_read_t fd = player->fd;
	batom_cursor c;
	ssize_t len;
	batom_vec vec;
	uint8_t buf[2/*wLongsPerEntry*/ + 1/* bIndexSubType */ + 1/* bIndexType */ + 4/* nEntriesInUse */ + 4/* dwChunkId */ + 8/* qwBaseOffset */];
    off_t seek_rc;
    off_t seek_target;

    if(stream->cur_superindex_entry < 0 || (unsigned)stream->cur_superindex_entry >= stream->superindex.nEntriesInUse) {
        return BERR_TRACE(-1);
    }
    seek_target = stream->superindex.avi_superindex_entry[stream->cur_superindex_entry].qwOffset + 8;
    seek_rc = fd->seek(fd, seek_target, SEEK_SET);
    if(seek_rc!=seek_target) {
        return BERR_TRACE(-1);
    }
	len = fd->read(fd, buf, sizeof(buf));
	if(len!=sizeof(buf)) {
		return BERR_TRACE(-1);
	}

	batom_vec_init(&vec, buf, len);
	batom_cursor_from_vec(&c, &vec, 1);

	stream->stdindex.wLongsPerEntry =  bavi_read_word(&c);
	stream->stdindex.bIndexSubType = bavi_read_byte(&c);
	stream->stdindex.bIndexType = bavi_read_byte(&c);
	stream->stdindex.nEntriesInUse =  bavi_read_dword(&c);
	stream->stdindex.dwChunkId = bavi_read_fourcc(&c);
	stream->stdindex.qwBaseOffset = bavi_read_qword(&c);

	BDBG_WRN(("wLongsPerEntry : %u, bIndexSubType : %u, bIndexType : %u,  nEntriesInUse : %u, dwChunkId " BMEDIA_FOURCC_FORMAT ", qwBaseOffset : %#lx",
		  stream->stdindex.wLongsPerEntry, stream->stdindex.bIndexSubType, stream->stdindex.bIndexType, stream->stdindex.nEntriesInUse, BMEDIA_FOURCC_ARG(stream->stdindex.dwChunkId),
		  (unsigned long)stream->stdindex.qwBaseOffset));

	if(BATOM_IS_EOF(&c)) {
		return BERR_TRACE(-1);
	}

	return 0;
}

static bavi_parser_action
b_avi_player_strl(bavi_parser_handler *handler, bavi_fourcc fourcc, batom_t object)
{
	batom_cursor c;
	bavi_player_t player = ((b_avi_player_object_handler*)handler)->player;
	BDBG_OBJECT_ASSERT(player, bavi_player_t);
	BDBG_MSG(("b_avi_player_strl: " BMEDIA_FOURCC_FORMAT "[" BMEDIA_FOURCC_FORMAT "] %u bytes", BMEDIA_FOURCC_ARG(handler->fourcc), BMEDIA_FOURCC_ARG(fourcc), object?(unsigned)batom_len(object):0));
	if(fourcc==BAVI_FOURCC_BEGIN) {
		player->stream_no++;
	}
	if(bmedia_player_stream_test(&player->stream, player->stream_no)) {
		b_avi_player_stream *stream;

		if(fourcc==BAVI_FOURCC_BEGIN) {
			stream = BKNI_Malloc(sizeof(*stream));
			if(stream) {
				stream->stream_no = player->stream_no;
				stream->header_valid = false;
				stream->stream_info_valid = false;
				stream->index = NULL;
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
                                player->vchunk_fourccs[0] = BMEDIA_FOURCC(strm_no_0,strm_no_1,'d','c');
                                player->vchunk_fourccs[1] = BMEDIA_FOURCC(strm_no_0,strm_no_1,'d','b');
                                player->vchunk_fourccs[2] = BMEDIA_FOURCC(strm_no_0,strm_no_1,'s','b');
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
					b_avi_read_superindex(stream, object);
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
b_avi_player_odml(bavi_parser_handler *handler, bavi_fourcc fourcc, batom_t object)
{
	bavi_player_t player = ((b_avi_player_object_handler*)handler)->player;
	bavi_dmlh dmlh;

	BDBG_MSG(("b_avi_player_odml: %p " BMEDIA_FOURCC_FORMAT "[" BMEDIA_FOURCC_FORMAT "] %u bytes", (void *)player, BMEDIA_FOURCC_ARG(handler->fourcc), BMEDIA_FOURCC_ARG(fourcc), object?(unsigned)batom_len(object):0));
	BDBG_OBJECT_ASSERT(player, bavi_player_t);
	switch(fourcc) {
	case BAVI_FOURCC_BEGIN:
		break;
	case BMEDIA_FOURCC('d','m','l','h'):
		if(!bavi_read_dmlh(&dmlh, object)) {
			BDBG_WRN(("b_avi_player_odml: %#lx error in the odml header", (unsigned long)player));
			goto done;
		}
		player->dmlh_found = true;
		if(player->mainheader_valid) {
			player->bounds.last = B_AVI_PLAYER_SCALE(dmlh.dwTotalFrames);
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
b_avi_player_object_begin(void *cntx, bavi_fourcc fourcc, bavi_dword size, bavi_off_t offset)
{
	bavi_player_t player = cntx;
    uint32_t i;

	BDBG_MSG(("b_avi_player_object_begin: %#lx " BMEDIA_FOURCC_FORMAT " %u bytes at %#lx", (unsigned long)cntx, BMEDIA_FOURCC_ARG(fourcc), size, (unsigned long)offset));
	BDBG_OBJECT_ASSERT(player, bavi_player_t);
	switch(fourcc) {
	case BMEDIA_FOURCC('m','o','v','i'):
			player->movi_off = offset;
			player->movi_size = size;
		break;
	case BMEDIA_FOURCC('i','d','x','1'):
		size = size - (size%B_AVI_OLD_INDEX_ENTRY);
		if(size>0 && player->dmf2_indx==false) {
			bfile_segment_set(&player->idx, offset, size);
		}
		player->idx1_off = offset;
		player->idx1_size = size;
		break;
	default:
		for(i=0;i<sizeof(player->vchunk_fourccs)/sizeof(player->vchunk_fourccs[0]);i++) {
			if(player->vchunk_fourccs[i] == fourcc) {
                 /* If we have hit a video chunk set the frame offset.  Adjust by 8 bytes to 
                    include 4 bytes for the FourCC and 4 bytes for the chunk size */
                if (offset >= 8) {
                    player->first_vframe_off = offset-8;
		            player->first_vframe_size = size+8;
                }
                else {
	                player->first_vframe_off = 0;
                }
                return bavi_parser_action_return;
			}
		}
		return bavi_parser_action_none;
	}
	return bavi_parser_action_return;
}

static int
b_avi_load_std_index(bavi_player_t player, b_avi_player_stream  *stream, bool next)
{
	bfile_segment *idx;
	bfile_cache_t idx_cache;
	int rc;

	if(stream->status.stream_type == bavi_stream_type_video) {
		idx = &player->idx;
		idx_cache = player->idx_cache;
	} else {
		idx = &player->dmf2_info.dmf2_idx[0];
		idx_cache = player->dmf2_info.dmf2_idx_cache[0];
	}

	if(next) {
		stream->cur_superindex_entry++;

		if(stream->cur_superindex_entry >= (int)stream->superindex.nEntriesInUse) {
			stream->cur_superindex_entry = (int)stream->superindex.nEntriesInUse-1;
			return -1;
		}
	} else {
		stream->cur_superindex_entry--;

		if(stream->cur_superindex_entry < 0) {
			stream->cur_superindex_entry = 0;
			return -1;
		}
	}

	rc = b_avi_read_stdindex(player, stream);
	if(rc<0) {
		return BERR_TRACE(rc);
	}
	bfile_segment_set(idx, (stream->superindex.avi_superindex_entry[stream->cur_superindex_entry].qwOffset + B_AVI_STD_INDEX_OFFSET), (stream->stdindex.nEntriesInUse*B_AVI_STD_INDEX_ENTRY));


	bfile_cache_clear(idx_cache);
	bfile_segment_reset(idx_cache, idx);
	rc = bfile_cache_seek(idx_cache, 0);
	if(rc<0) {
		BDBG_WRN(("b_avi_load_std_index:< %#lx can't access index %u", (unsigned long)player, 0));
		rc=BERR_TRACE(rc);
	}


	return rc;
}

static int
b_avi_seek_std_index(bavi_player_t player, unsigned index_pos)
{
	b_avi_player_stream  *stream;
	int rc;

	stream = b_avi_player_get_video_stream(player);

	if((index_pos>=player->dmf2_info.dmf2_index_acc) && (index_pos<(stream->stdindex.nEntriesInUse+player->dmf2_info.dmf2_index_acc-1))) {
		return 0;
	} else if(index_pos<player->dmf2_info.dmf2_index_acc) {
		while(index_pos<player->dmf2_info.dmf2_index_acc) {
			rc = b_avi_load_std_index(player, stream, false);
			if(rc<0) {
				return BERR_TRACE(-1);
			}
			player->dmf2_info.dmf2_index_acc -= stream->stdindex.nEntriesInUse;
		}
	} else if(index_pos>(stream->stdindex.nEntriesInUse+player->dmf2_info.dmf2_index_acc-1)) {
		while(index_pos>(stream->stdindex.nEntriesInUse+player->dmf2_info.dmf2_index_acc-1)) {
			player->dmf2_info.dmf2_index_acc += stream->stdindex.nEntriesInUse;
			rc = b_avi_load_std_index(player, stream, true);
			if(rc<0) {
				return BERR_TRACE(-1);
			}
		}
	}

	return 0;
}

static int
b_avi_player_open_index(bavi_player_t player)
{
	batom_factory_t factory;
	batom_pipe_t pipe_avi;
	batom_t atom;
	bfile_io_read_t fd;
	bavi_parser_cfg cfg;
	bavi_parser_t  avi;
	enum {b_avi_player_open_state_movi, b_avi_player_open_state_vframe, b_avi_player_open_state_idx} state;
	int rc=-1;
	b_avi_player_stream *stream;
    size_t parsed_size;

	BDBG_OBJECT_ASSERT(player, bavi_player_t);
	state = b_avi_player_open_state_movi;
	fd = player->fd;
	player->movi_off = -1;
	player->movi_size = 0;
	player->first_vframe_off = -1;
	player->first_vframe_size = 0;
	bfile_segment_clear(&player->idx);
	player->mainheader_valid = false;
	player->stream_no = 0;
    player->index_error_cnt = 0;

	factory = batom_factory_create(bkni_alloc, 64);
	if(!factory) {
		goto err_factory;
	}

	bavi_parser_default_cfg(&cfg);
	cfg.user_cntx = player;
	cfg.object_begin = b_avi_player_object_begin;
	avi = bavi_parser_create(factory, &cfg);
	if(!avi) {
		goto err_avi;
	}
	player->hdrl_handler.player = player;
	player->strl_handler.player = player;
	player->odml_handler.player = player;
	bavi_parser_install_handler(avi, &player->hdrl_handler.handler, BMEDIA_FOURCC('h','d','r','l'), b_avi_player_hdrl);
	bavi_parser_install_handler(avi, &player->strl_handler.handler, BMEDIA_FOURCC('s','t','r','l'), b_avi_player_strl);
	bavi_parser_install_handler(avi, &player->odml_handler.handler, BMEDIA_FOURCC('o','d','m','l'), b_avi_player_odml);
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
		parsed_size += len;
		read_result =  fd->read(fd, buf, len);
		if(read_result<0) {
			BKNI_Free(buf);
			goto err_read;
		}
		atom = batom_from_range(factory, buf, read_result, &b_avi_player_atom, &buf);
		if(!atom) {
			BKNI_Free(buf);
			goto err_atom;
		}
		batom_pipe_push(pipe_avi, atom);
		avi_result = bavi_parser_feed(avi, pipe_avi);
		if(avi_result==0) {
			goto err_parser;
		}

		if(state == b_avi_player_open_state_movi && player->movi_off>=0) {
			off_t seek_result;

			/* located movi object */
			state = b_avi_player_open_state_vframe;
			batom_pipe_flush(pipe_avi);
			BDBG_MSG(("b_avi_player_open_index: seek to " BDBG_UINT64_FMT " movi (" BDBG_UINT64_FMT ")", BDBG_UINT64_ARG(player->movi_off), BDBG_UINT64_ARG(player->movi_off)));

			seek_result = fd->seek(fd, player->movi_off, SEEK_SET);
			if(seek_result != player->movi_off) {
				goto err_seek;
			}			
			bavi_parser_seek(avi, player->movi_off); /* skip movi object */
		} else if(state == b_avi_player_open_state_vframe && player->first_vframe_off>=0) {
			off_t idx_off;
			off_t seek_result;

			/* located the first video frame object */
			state = b_avi_player_open_state_idx;
			batom_pipe_flush(pipe_avi);
			if(player->dmf2_indx) {
				rc=0;
				seek_result = fd->seek(fd, player->movi_off, SEEK_SET);
				if(seek_result != player->movi_off) {
					goto err_seek;
				}			
				bavi_parser_seek(avi, player->movi_off); /* skip movi object */
				break;
			} else{
				idx_off =  player->movi_off + player->movi_size;
			}		
			BDBG_MSG(("b_avi_player_open_index: seek to " BDBG_UINT64_FMT " movi (" BDBG_UINT64_FMT ")", BDBG_UINT64_ARG(idx_off), BDBG_UINT64_ARG(player->movi_off)));

			seek_result = fd->seek(fd, idx_off, SEEK_SET);
			if(seek_result != idx_off) {
				goto err_seek;
			}
			bavi_parser_seek(avi, idx_off); /* skip movi object */
		} else if(state == b_avi_player_open_state_idx && bfile_segment_test(&player->idx)) {
            off_t idx_off;
            off_t seek_result;

			if(!player->dmf2_indx) {				
				idx_off =  player->idx1_off + player->idx1_size;
				seek_result = fd->seek(fd, 0, SEEK_END);			
				if(seek_result < idx_off) {
                    uint64_t idx_len = seek_result - player->idx1_off;
                    BDBG_WRN(("detected truncated index: %u<>%u",(unsigned)idx_off, (unsigned)seek_result));
                    if(seek_result > 0 && seek_result > player->idx1_off && idx_len > B_AVI_OLD_INDEX_ENTRY)  {
                        BDBG_WRN(("using %u(%u) avaliable index entries", (unsigned)(idx_len / B_AVI_OLD_INDEX_ENTRY), (unsigned)(player->idx.len / B_AVI_OLD_INDEX_ENTRY)));
                        player->idx.len = idx_len;
                    } else {
                        goto err_seek;
                    }
				}				
				player->idx_nentries = player->idx.len / B_AVI_OLD_INDEX_ENTRY;
			}

			if(player->mainheader_valid && player->idx_nentries>0) {
				rc = 0; /* have found location of the index */
			} break;
		}
	}

	if(rc!=0) {
		BDBG_WRN(("b_avi_player_open_index: %#lx no valid streams found", (unsigned long)player));
	}
	if(player->dmf2_indx) {
		b_avi_player_stream *stream;

		for(stream=BLST_SQ_FIRST(&player->streams);stream;stream=BLST_SQ_NEXT(stream, link)) {
			int index_rc;
			index_rc = b_avi_read_stdindex(player, stream);
			if(index_rc<0) { 
				rc=BERR_TRACE(index_rc);
				break;
			}
		}

		player->idx_nentries = player->idx.len / B_AVI_STD_INDEX_ENTRY;
		player->dmf2_info.dmf2_start_offset = player->movi_off;		
	}
	if(player->bounds.last==0) {
		player->bounds.last = B_AVI_PLAYER_SCALE(player->mainheader.dwTotalFrames);
	}
	BDBG_MSG(("b_avi_player_open_index: %#lx bounds:%u..%u", (unsigned long)player, (unsigned)player->bounds.first, (unsigned)player->bounds.last));

	for(stream=BLST_SQ_FIRST(&player->streams);stream!=NULL;) {
		/* coverity[deref_after_free] */
		b_avi_player_stream *next_stream = BLST_SQ_NEXT(stream, link);
		if(stream->header_valid && stream->stream_info_valid && stream->status.header.dwRate) {
			bmedia_player_pos duration = player->bounds.last;

			BDBG_MSG(("b_avi_player_open_index: %#lx stream_no:%u duration %u", (unsigned long)player, (unsigned)stream->stream_no, (unsigned)duration));
			stream->index = bmedia_index_create(duration, sizeof(b_avi_index_entry), 100);
		}
		if(!stream->index) {
			BDBG_WRN(("b_avi_player_open_index: %#lx invalid stream_no %u", (unsigned long)player, stream->stream_no));
			BLST_SQ_REMOVE(&player->streams, stream, b_avi_player_stream, link);
			if(player->dmf2_indx && stream->superindex.avi_superindex_entry){
				BKNI_Free(stream->superindex.avi_superindex_entry);
			}
			BKNI_Free(stream);
			stream = next_stream;
			continue;
		}
		if(stream->status.stream_type == bavi_stream_type_audio) {
			bavi_audio_state_set_header(&stream->audio, &stream->status.header, &stream->status.stream_info.audio);					
		}
		stream->index_wait_time = bmedia_index_next(stream->index);
		stream = next_stream;
	}

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
b_avi_player_free_streams(bavi_player_t player)
{
	b_avi_player_stream *stream;
	while(NULL!=(stream=BLST_SQ_FIRST(&player->streams))) {
		BLST_SQ_REMOVE_HEAD(&player->streams, link);
		if(stream->index) {
			bmedia_index_destroy(stream->index);
		}
		if(player->dmf2_indx && stream->superindex.avi_superindex_entry){
			BKNI_Free(stream->superindex.avi_superindex_entry);
		}
		BKNI_Free(stream);
	}
	return;
}


bavi_player_t
bavi_player_create(bfile_io_read_t fd, const bmedia_player_config *config, const bmedia_player_stream *stream)
{
	bavi_player_t player;
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
	BDBG_OBJECT_INIT(player, bavi_player_t);
	player->fd = fd;
	player->config = *config;
	player->stream = *stream;
	BLST_SQ_INIT(&player->streams);
	strm_no[0] = '0' | (((stream->master-1)/10)%10);
	strm_no[1] = '0' | ((stream->master-1)%10);
	player->stream_fourcc = BMEDIA_FOURCC(strm_no[0], strm_no[1], 'd', 'c'); /* XXX video stream only */
	player->encrypted_video = false;
	player->dmf2_indx=false;
	player->dmf2_info.dmf2_index_acc = 0;
	player->dmf2_info.curr_dmf2_riff_indx = 0;
	BKNI_Memset(player->dmf2_info.dmf2_idx_cache, 0, sizeof(player->dmf2_info.dmf2_idx_cache));
	player->bounds.first = 0;
	player->bounds.last = 0;
	player->vchunk_fourccs[0] = player->stream_fourcc;
	player->vchunk_fourccs[1] = 0;
	player->vchunk_fourccs[2] = 0;
	player->video_stream = NULL;
	player->preframe_required = false;

	if(b_avi_player_open_index(player)<0) {
		BDBG_WRN(("no index"));
		goto err_index;
	}
	if(!BLST_SQ_FIRST(&player->streams)) {
		BDBG_WRN(("no streams"));
		goto err_index;
	}

	if(player->dmf2_indx) {
		b_avi_player_stream *list;
		int cnt=0;

		for(list=BLST_SQ_FIRST(&player->streams);list;list=BLST_SQ_NEXT(list, link)) {
			if(list->status.stream_type == bavi_stream_type_video) {
				bfile_segment_set(&player->idx, (list->superindex.avi_superindex_entry[list->cur_superindex_entry].qwOffset + B_AVI_STD_INDEX_OFFSET), (list->stdindex.nEntriesInUse*B_AVI_STD_INDEX_ENTRY));
				player->idx_cache = bfile_cache_create(fd, &player->idx, B_AVI_STD_INDEX_CACHE, B_AVI_STD_INDEX_ENTRY);
			}
			else if(list->status.stream_type == bavi_stream_type_audio) {
				bfile_segment_set(&player->dmf2_info.dmf2_idx[cnt], (list->superindex.avi_superindex_entry[list->cur_superindex_entry].qwOffset + B_AVI_STD_INDEX_OFFSET), (list->stdindex.nEntriesInUse*B_AVI_STD_INDEX_ENTRY));
				player->dmf2_info.dmf2_idx_cache[cnt] = bfile_cache_create(fd, &player->dmf2_info.dmf2_idx[cnt], B_AVI_STD_INDEX_CACHE, B_AVI_STD_INDEX_ENTRY);
				cnt++;
			}
		}
	} else {
		player->idx_cache = bfile_cache_create(fd, &player->idx, B_AVI_OLD_INDEX_CACHE, B_AVI_OLD_INDEX_ENTRY);
	}

	if(!player->idx_cache) {
		goto err_cache;
	}


	player->pos = 0;
	player->step = 0;
	player->player_state = b_avi_player_state_header;
	player->last_read_data_offset = player->movi_off + player->movi_size;

	player->chunk_size = 60 * 1024;
	player->embedded_length = 0;
	player->need_timestamp = true;
	player->eof_reached = false;
	player->dmlh_found = false;
	player->zero[0] = 0;
	BKNI_Memset(player->dmf2_info.dmf2_resync_offset, 0, sizeof(player->dmf2_info.dmf2_resync_offset));	

	if(player->dmf2_indx) {		
		player->dmf2_info.riff_info[0].start = 0;
		player->dmf2_info.riff_info[0].len = player->last_read_data_offset;
		player->dmf2_info.riff_info[0].movi_off = player->movi_off;
	}

	buf = bfile_cache_next(player->idx_cache);
	if(!buf) {
		goto err_index_off;
	}

	if(player->dmf2_indx) {
		player->index_movi_off = 0;
	} else {
		if(B_AVI_GET_CHUNK_OFF(buf)+4/*LIST*/+4/*Length*/+4/*movi*/ < player->movi_off) {
			player->index_movi_off = player->movi_off;
		} else {
			BDBG_WRN(("bavi_player_create: %#lx index uses absolute offsets %u:%u", (unsigned long)player, (unsigned)B_AVI_GET_CHUNK_OFF(buf)+4+4+4, (unsigned)player->movi_off));
			player->index_movi_off = 4;
		}
	}
	rc = bfile_cache_seek(player->idx_cache, 0);
	if(rc<0) {
		goto err_cache;
	}

	return player;

err_index_off:
	bfile_cache_destroy(player->idx_cache);
err_cache:
err_index:
	b_avi_player_free_streams(player);
	BKNI_Free(player);
err_alloc:
	return NULL;
}

void
bavi_player_destroy(bavi_player_t player)
{
	unsigned cnt;

	BDBG_OBJECT_ASSERT(player, bavi_player_t);

	for(cnt=0;cnt<sizeof(player->dmf2_info.dmf2_idx_cache)/sizeof(player->dmf2_info.dmf2_idx_cache[0]);cnt++) {
		if (player->dmf2_info.dmf2_idx_cache[cnt]) {
			bfile_cache_destroy(player->dmf2_info.dmf2_idx_cache[cnt]);
		}
	}

	bfile_cache_destroy(player->idx_cache);
	b_avi_player_free_streams(player);
	BDBG_OBJECT_DESTROY(player, bavi_player_t);
	BKNI_Free(player);
	return;
}

static void
b_avi_player_index_point(bavi_player_t player, b_avi_player_stream *stream, bmedia_player_pos time, const uint8_t *buf)
{
	BSTD_UNUSED(player);
	if(stream->index_wait_time != BMEDIA_PLAYER_INVALID && time >= stream->index_wait_time) {
        bool is_seekable_point = ((stream->status.stream_type != bavi_stream_type_video) || (player->dmf2_indx?B_AVI_IS_STD_KEYFRAME(buf):B_AVI_IS_KEYFRAME(buf)) || time==player->bounds.first);
        if(!is_seekable_point) {
            return;
        }
		BDBG_MSG(("b_avi_player_index_point: %p saving index (%u:%u) stream:%u frame:%u size:%u pos:%u", (void *)player, (unsigned)time, (unsigned)stream->index_wait_time, stream->stream_no, stream->index_entry.frame_no, (unsigned)stream->index_entry.size, stream->index_entry.pos));
		if(stream->status.stream_type==bavi_stream_type_audio) {
			bavi_audio_state_get_memo(&stream->audio, &stream->index_entry.audio_memo);
		}
		bmedia_index_add(stream->index, time, &stream->index_entry);
		stream->index_wait_time = bmedia_index_next(stream->index);
	}
	return;
}


static bmedia_player_pos
b_avi_player_stream_time(b_avi_player_stream *stream)
{
	bmedia_player_pos time;

	if(stream->status.stream_type == bavi_stream_type_video) {
		time = (uint32_t)(((stream->status.header.dwStart+stream->index_entry.frame_no) * ((uint64_t)stream->status.header.dwScale*1000)/stream->status.header.dwRate));
	} else {
		time = bavi_audio_state_get_timestamp(&stream->audio, stream->index_entry.frame_no, stream->index_entry.size);
	}
	return time;
}

static b_avi_player_stream *
b_avi_player_get_stream(bavi_player_t player, unsigned stream_no)
{
	b_avi_player_stream *stream;
	for(stream=BLST_SQ_FIRST(&player->streams);stream;stream=BLST_SQ_NEXT(stream, link)) {
		if(stream->stream_no==stream_no) {
			break;
		}
	}
	return stream;
}

static void
b_avi_player_update_audio_stream(bavi_player_t player, b_avi_player_stream *stream, const uint8_t *buf)
{
    BDBG_ASSERT(buf);
	if(stream->status.stream_type == bavi_stream_type_audio) {
        bool dmf2 = player->dmf2_indx;
        size_t obj_size = (dmf2?B_AVI_GET_STD_OBJ_SIZE(buf):B_AVI_GET_OBJ_SIZE(buf));
		bavi_audio_state_update(&stream->audio, obj_size);
	}
}

static void
b_avi_player_update_stream(bavi_player_t player, b_avi_player_stream *stream, const uint8_t *buf)
{
    bool dmf2 = player->dmf2_indx;
	size_t obj_size;

    BDBG_ASSERT(buf);
	obj_size = (dmf2?B_AVI_GET_STD_OBJ_SIZE(buf):B_AVI_GET_OBJ_SIZE(buf));
	stream->index_entry.frame_no++;
	stream->index_entry.size += obj_size;
	return;
}

static b_avi_player_stream *
b_avi_player_get_video_stream(bavi_player_t player)
{
	b_avi_player_stream *stream = player->video_stream;
    if (!stream) {
	    for(stream=BLST_SQ_FIRST(&player->streams);stream;stream=BLST_SQ_NEXT(stream, link)) {
		    if(stream->status.stream_type == bavi_stream_type_video) {
			    break;
		    }
	    }
    }
	return stream;
}

static int
b_avi_load_next_riff_info(bavi_player_t player)
{
	off_t cur_pos;

	cur_pos = player->fd->seek(player->fd, 0, SEEK_CUR);
	player->fd->seek(player->fd, player->last_read_data_offset, SEEK_SET);
	if(player->fd->read(player->fd, player->embedded_data, B_AVIX_HEADER_SIZE) != B_AVIX_HEADER_SIZE){
		return BERR_TRACE(-1);
	}

	/* Check if we really reached the end of riff. Some AVI 2.0 files may have AVI 1.0 index after movi chunk */
	if(BMEDIA_FOURCC(player->embedded_data[0], player->embedded_data[1], player->embedded_data[2], player->embedded_data[3]) == BMEDIA_FOURCC('i', 'd', 'x', '1')){
        off_t seek_target = B_MEDIA_LOAD_UINT32_LE(player->embedded_data,4) - 16;
        off_t seek_rc;
		player->last_read_data_offset += B_MEDIA_LOAD_UINT32_LE(player->embedded_data,4) + 8;
        seek_rc = player->fd->seek(player->fd, seek_target, SEEK_CUR);
        if(seek_rc != seek_target) {
            return BERR_TRACE(-1);
        }
		if(player->fd->read(player->fd, player->embedded_data, B_AVIX_HEADER_SIZE)!= B_AVIX_HEADER_SIZE){
			return BERR_TRACE(-1);
		}
	}

	player->dmf2_info.curr_dmf2_riff_indx++;
	player->dmf2_info.riff_info[player->dmf2_info.curr_dmf2_riff_indx].start = player->last_read_data_offset;
	player->dmf2_info.riff_info[player->dmf2_info.curr_dmf2_riff_indx].len = B_AVI_GET_STD_CHUNK_SIZE(player->embedded_data);
	player->dmf2_info.riff_info[player->dmf2_info.curr_dmf2_riff_indx].movi_off = 24;

	player->last_read_data_offset += B_AVI_GET_STD_CHUNK_SIZE(player->embedded_data);

	player->fd->seek(player->fd, cur_pos, SEEK_SET);

	return 0;
}

static int
b_avi_seek_riff(bavi_player_t player, bmedia_player_entry *entry, bavi_off_t start, unsigned index_pos)
{
	bool reseek = false;
	int rc; 

	if(start >= player->last_read_data_offset) {
		rc = b_avi_load_next_riff_info(player);
		if(rc<0) {
			return BERR_TRACE(rc);
		}
		reseek = true;
	} else if(start < player->dmf2_info.riff_info[player->dmf2_info.curr_dmf2_riff_indx].start) {
		player->last_read_data_offset = player->dmf2_info.riff_info[player->dmf2_info.curr_dmf2_riff_indx].start;
		player->dmf2_info.curr_dmf2_riff_indx--;
		reseek = true;
	}
	if(reseek) {		
		unsigned index = (index_pos-player->dmf2_info.dmf2_index_acc)*B_AVI_STD_INDEX_ENTRY;
		entry->start =  0 ;
		entry->length = player->movi_off;
		entry->embedded = NULL;
		entry->content = bmedia_player_entry_content_header;
		player->player_state = b_avi_player_state_avix_header;

		rc = bfile_cache_seek(player->idx_cache, index);
		if(rc<0) {
			BDBG_WRN(("b_avi_seek_riff:< %#lx can't access index %u", (unsigned long)player, index));
			return BERR_TRACE(rc);
		}

		player->pos = index_pos;
		return 0;
	}

	return -1;
}

static int
b_avi_player_next_play(bavi_player_t player, bmedia_player_entry *entry)
{
	b_avi_player_stream *stream, *video_stream, *list;
	bool valid_block;
	bavi_off_t block_begin=0,block_end=0;
	unsigned index_pos;
	unsigned block_count, cnt, i;

	video_stream = b_avi_player_get_video_stream(player);

	for(block_count=0,index_pos=player->pos,valid_block=false;;) {
		bavi_off_t start;
		const uint8_t *buf = bfile_cache_next(player->idx_cache);
		bmedia_player_pos time;
		bool drop = false;

		if(buf==NULL) {
            if(bfile_cache_is_file_error(player->idx_cache)) {
				BDBG_WRN(("b_avi_player_next_play:< %#lx can't read index %u", (unsigned long)player, index_pos));
				return -1;
            } else if(!valid_block)  {
				if(player->dmf2_indx) {
					if(b_avi_load_std_index(player, video_stream, true)==0) {
						player->dmf2_info.dmf2_index_acc = index_pos;
						continue;
					} 
				} else {
                    entry->type = bmedia_player_entry_type_end_of_stream;
                    goto done;
                }
			}
			break;
		}

		index_pos++;
        
		/* If this index points to a chunk from a track we do not wish to ouput then skip it */
		for(i = 0; player->stream.drop[i] && !drop && (i < BMEDIA_PLAYER_MAX_TRACKS); i++) {
			drop = (player->stream.drop[i] == B_AVI_GET_STREAM_ID(buf));
		}
		if (drop) continue;

		if(player->dmf2_indx)
			stream=video_stream;
		else
			stream = b_avi_player_get_stream(player, B_AVI_GET_STREAM_ID(buf));
		
		if(!stream) {
			if(!player->dmf2_indx && !valid_block && B_AVI_IS_SUBTITLE_HEADER(buf)){
					block_begin = B_AVI_GET_CHUNK_OFF(buf);
					valid_block = true;
			}
			continue;
		}

		if(stream->index_entry.pos>index_pos) {
			if(valid_block) {
				BDBG_MSG_TRACE(("b_avi_player_next_play: %#lx skip stream:%u (%u:%u) block:%u:%u", (unsigned long)player, stream->stream_no, index_pos, stream->index_entry.pos, (unsigned)block_begin, (unsigned)(block_end-block_begin)));
				break;
			} else {
				continue;
			}
		}

		if(player->dmf2_indx) {
			start = B_AVI_GET_STD_CHUNK_OFF(buf) + stream->stdindex.qwBaseOffset;
            if (start < (bavi_off_t)stream->stdindex.qwBaseOffset) {

                /* This index entry has an invalid size, construct a video dummy frame of size 0 */
                player->index_error_cnt++;
	            player->config.error_detected(player->config.cntx);
                player->embedded_data[0] = '0' | ((stream->stream_no-1)/10)%10;
                player->embedded_data[1] = '0' | (stream->stream_no-1)%10;
                player->embedded_data[2] = 'd';
                player->embedded_data[3] = 'c';
                player->embedded_data[4] = 0;
                player->embedded_data[5] = 0;
                player->embedded_data[6] = 0;
                player->embedded_data[7] = 0;
	            player->embedded_length = 8;

	            b_avi_player_update_stream(player, stream, buf);
		        if(valid_block) {
                    /* If there is already a vaild block, we will finish it and send the dummy frame with the next entry.
                       There is a potential issue here if the player state is switched later in this function. Without a queue, 
                       I'm not sure of a good way around this. */
    		        player->player_state = b_avi_player_state_embedded;
                    player->last_read_data_offset = player->movi_off;
                    break;
                }

                /* If there is not a valid block we must send the dummy frame now */
                entry->start =  player->movi_off;
                entry->length = player->embedded_length;
                entry->embedded = player->embedded_data;
                entry->type = bmedia_player_entry_type_embedded;
                entry->content = bmedia_player_entry_content_payload;
                player->player_state = b_avi_player_state_data;
                goto done;
            }

			if(player->need_timestamp) {
				if(b_avi_seek_riff(player, entry, start, index_pos--)==0) {
                    goto done;
				} else {
					index_pos++;
				}
			}
		} else {
			start = B_AVI_GET_CHUNK_OFF(buf);
		}

		stream->index_entry.pos = index_pos-1;
		time = b_avi_player_stream_time(stream);
		BDBG_MSG_TRACE(("b_avi_player_next_play: %p stream:%u (%u:%u) time:%u) pos:%u", (void *)player, stream->stream_no, stream->index_entry.frame_no, (unsigned)stream->index_entry.size, (unsigned)time, index_pos-1));

		b_avi_player_index_point(player, stream, time, buf);

		if(!valid_block) {
			if(!player->need_timestamp) {
				entry->timestamp = 0;
				entry->entry = NULL;
			} else {
				player->need_timestamp  = false;
				entry->timestamp = time;
				BKNI_Memset(&player->resync, 0, sizeof(bmedia_player_sync_entry));

				for(cnt=1, list=BLST_SQ_FIRST(&player->streams);list;list=BLST_SQ_NEXT(list, link)) {
					if(list==video_stream) {
						/* Always assign video stream to index 0*/
						player->resync.resync[0].stream_id = list->stream_no;
						player->resync.resync[0].timestamp = (player->dmf2_indx?time:b_avi_player_stream_time(list));
					}
					else{
						player->resync.resync[cnt].stream_id = list->stream_no;
						player->resync.resync[cnt].timestamp = (player->dmf2_indx?(time + player->dmf2_info.dmf2_resync_offset[0/* cnt-1 */]):b_avi_player_stream_time(list));
						cnt++;
					}
				}
                entry->entry = &player->resync;
			}
			block_begin = start;
			if(player->dmf2_indx && (block_begin>player->dmf2_info.dmf2_start_offset)) {
				block_begin = player->dmf2_info.dmf2_start_offset;
			}
			valid_block = true;
		}

		if(!B_AVI_IS_DRM_HEADER(buf)) {
			b_avi_player_update_audio_stream(player, stream, buf);
			b_avi_player_update_stream(player, stream, buf);
		}

		block_end = start + (player->dmf2_indx?B_AVI_GET_STD_CHUNK_SIZE(buf):B_AVI_GET_CHUNK_SIZE(buf));
		block_count++;
		if(block_end >= block_begin + (int)player->chunk_size || block_count>=30) {
			BDBG_MSG_TRACE(("b_avi_player_next_play: %#lx completed block %u:%u(%u) block_count:%u", (unsigned long)player, (unsigned)block_begin, (unsigned)(block_end-block_begin), (unsigned)player->chunk_size, block_count));
			break;
		}
	}

	if(!valid_block) {
        entry->type = bmedia_player_entry_type_end_of_stream;
		BDBG_MSG_TRACE(("b_avi_player_next_play: %#lx EOF %u", (unsigned long)player, index_pos));
		return 0;
	}

    if(player->dmf2_indx) {
		player->dmf2_info.dmf2_start_offset = block_end;
		if((block_end - block_begin) > DMF2_SEGMENT_SIZE){
            if (player->player_state != b_avi_player_state_data) {
                BDBG_WRN(("b_avi_player_next_play: %#lx Moving player from state %u to state dmf2 segment! This might cause unexpected behavior!", (unsigned long)player, (unsigned)player->player_state));
            }
			player->dmf2_info.segment.end = block_end;
			block_end = block_begin + DMF2_SEGMENT_SIZE;
			player->dmf2_info.segment.start = block_end;
			player->player_state = b_avi_player_state_dmf2_segment;
		}

	}

	entry->start = block_begin+player->index_movi_off;
	entry->length = block_end - block_begin;
	entry->embedded = NULL;
	entry->atom = NULL;
	entry->type = bmedia_player_entry_type_file;

done:
	player->pos = index_pos;
	return 0;
}

static int
b_avi_player_next_forward(bavi_player_t player, bmedia_player_entry *entry)
{
	b_avi_player_stream *video_stream;
	unsigned index_pos;
	bmedia_player_pos dest_time;
	const uint8_t *buf;
	b_avi_player_stream *stream;
	bmedia_player_pos time;

	BKNI_Memset(&player->resync, 0, sizeof(bmedia_player_sync_entry));

	video_stream = b_avi_player_get_video_stream(player);
	if(!video_stream) {
		BDBG_WRN(("b_avi_player_next_forward: %#lx no video stream", (unsigned long)player));
		return -1;
	}
	dest_time = b_avi_player_stream_time(video_stream);
	if(!video_stream->seek_completed) {
		dest_time += player->step;
	}
	index_pos = player->pos;
	stream = video_stream;
	/* since we want to index _all_ streams when doing fast forward we would need to syncrhonize all streams first */
	if(index_pos<video_stream->index_entry.pos) {
		for(;;) {
			buf = bfile_cache_next(player->idx_cache);
			if(!buf) {
			    BDBG_MSG(("b_avi_player_next_forward: %#lx EOF in sync %u %s", (unsigned long)player, index_pos, bfile_cache_is_file_error(player->idx_cache)?"file_error":""));
				if(bfile_cache_is_file_error(player->idx_cache)) {
				    return -1;
                }
                entry->type = bmedia_player_entry_type_end_of_stream;
                return 0;
			}
			index_pos++;
			if(!player->dmf2_indx) {
				stream = b_avi_player_get_stream(player, B_AVI_GET_STREAM_ID(buf));
				if(!stream || B_AVI_IS_DRM_HEADER(buf)) {
					continue;
				}
			}

			if(index_pos <= stream->index_entry.pos) {
				continue;
			}
			stream->seek_completed = false;
			stream->index_entry.pos = index_pos;
			time = b_avi_player_stream_time(stream);
			BDBG_MSG_TRACE(("b_avi_player_next_forward: %p presync stream:%u (%u:%u) time:%u(%u) pos:%u", (void *)player, stream->stream_no, stream->index_entry.frame_no, (unsigned)stream->index_entry.size, (unsigned)time, (unsigned)dest_time, index_pos));
			b_avi_player_index_point(player, stream, time, buf);
			stream->index_entry.frame_no++;
			stream->index_entry.size += (player->dmf2_indx?B_AVI_GET_STD_OBJ_SIZE(buf):B_AVI_GET_OBJ_SIZE(buf));
			break;
		}
	}

	for(;;index_pos++) {
		buf = bfile_cache_next(player->idx_cache);
		if(!buf) {
			if(bfile_cache_is_file_error(player->idx_cache)) {
				return -1;
			}
			if(player->dmf2_indx) {
				if(b_avi_load_std_index(player, video_stream, true)==0) {
					player->dmf2_info.dmf2_index_acc = index_pos--;
					continue;
				}
			} 
            player->pos = index_pos-1;
            entry->type = bmedia_player_entry_type_end_of_stream;
            BDBG_MSG(("b_avi_player_next_forward: %#lx EOF %u", (unsigned long)player, index_pos));
            return 0;
		}
		if(!player->dmf2_indx) {
 			stream = b_avi_player_get_stream(player, B_AVI_GET_STREAM_ID(buf));
			if(!stream || B_AVI_IS_DRM_HEADER(buf)) {
				continue;
			}
		}
		if(index_pos < stream->index_entry.pos) {
			continue;
		}
		stream->seek_completed = false;
		stream->index_entry.pos = index_pos;
		time = b_avi_player_stream_time(stream);
		BDBG_MSG_TRACE(("b_avi_player_next_forward: %p stream:%u (%u:%u) time:%u(%u) pos:%u %s", (void *)player, stream->stream_no, stream->index_entry.frame_no, (unsigned)stream->index_entry.size, (unsigned)time, (unsigned)dest_time, index_pos, (player->dmf2_indx?B_AVI_IS_STD_KEYFRAME(buf):B_AVI_IS_KEYFRAME(buf))?"KEY":""));
		b_avi_player_index_point(player, stream, time, buf);
		if(stream==video_stream && time>=dest_time && (player->dmf2_indx?B_AVI_IS_STD_KEYFRAME(buf):B_AVI_IS_KEYFRAME(buf))) {
			int rc;
			bavi_off_t start;
			if(player->dmf2_indx) {
				start = B_AVI_GET_STD_CHUNK_OFF(buf) + stream->stdindex.qwBaseOffset;
				if(b_avi_seek_riff(player, entry, start, index_pos)==0) {
					return 0;
				}
			} else {
				start = B_AVI_GET_CHUNK_OFF(buf);
			}

			/* Always assign video stream to index 0 */
			player->resync.resync[0].stream_id = stream->stream_no;
			player->resync.resync[0].timestamp = time;
			entry->entry = &player->resync;
			entry->timestamp = time;
			entry->start = start+player->index_movi_off;
			/* 4 + 2 is used to account for the stream ID and object length needed by the AVI parser */
			entry->length = (player->dmf2_indx?B_AVI_GET_STD_CHUNK_SIZE(buf):B_AVI_GET_CHUNK_SIZE(buf));
			entry->embedded = NULL;
			entry->type = bmedia_player_entry_type_file;
			if(player->encrypted_video) {
				if(!player->dmf2_indx) {
					entry->start -= B_AVI_DRM_HEADER_SIZE;
					entry->length += B_AVI_DRM_HEADER_SIZE;
				} else {
					buf = bfile_cache_next(player->idx_cache);
					if(!buf) {
						BDBG_MSG(("b_avi_player_next_forward: %#lx frame EOF %u %s", (unsigned long)player, index_pos, bfile_cache_is_file_error(player->idx_cache)?"file_error":""));
						return -1;
					}
					start = B_AVI_GET_STD_CHUNK_OFF(buf) + stream->stdindex.qwBaseOffset;
					entry->length = (start-entry->start);
				}
			}
			player->pos = index_pos;
			BDBG_MSG(("b_avi_player_next_forward: %#lx found frame(%u:%u) %u:%u(%u)", (unsigned long)player, stream->index_entry.frame_no, (unsigned)time, (unsigned)entry->start, (unsigned)entry->length, (unsigned)index_pos));

			if(!player->dmf2_indx) {
				rc = bfile_cache_seek(player->idx_cache, index_pos*B_AVI_OLD_INDEX_ENTRY);
			} else {
				rc = bfile_cache_seek(player->idx_cache, (index_pos-player->dmf2_info.dmf2_index_acc)*B_AVI_STD_INDEX_ENTRY);
			}

			if(rc<0) {
				BDBG_WRN(("b_avi_player_next_forward:< %#lx can't access index %u", (unsigned long)player, index_pos));
			}
			return 0;
		}
        b_avi_player_update_audio_stream(player, stream, buf);
		b_avi_player_update_stream(player, stream, buf);
	}
}

static int
b_avi_player_next_backward(bavi_player_t player, bmedia_player_entry *entry)
{
	b_avi_player_stream *video_stream;
	unsigned index_pos;
	bmedia_player_pos dest_time;
	bmedia_player_pos time;
	int rc;

	BKNI_Memset(&player->resync, 0, sizeof(bmedia_player_sync_entry));

	video_stream = b_avi_player_get_video_stream(player);
	if(!video_stream) {
		BDBG_WRN(("b_avi_player_next_backward: %#lx no video stream", (unsigned long)player));
		return -1;
	}
	time = b_avi_player_stream_time(video_stream);
    if(!video_stream->seek_completed) {
        dest_time = time+player->step;  /* step is negative here */
        if(dest_time > time) {
            BDBG_MSG(("b_avi_player_next_backward: %p EOF %u:%d", (void *)player, (unsigned)time, (int)player->step));
            entry->type = bmedia_player_entry_type_end_of_stream;
            return 0;
        }
    } else {
        dest_time = time;
    }
	for(index_pos=video_stream->index_entry.pos;;index_pos--) {
		const uint8_t *buf;
		b_avi_player_stream *stream;

		if(player->dmf2_indx && index_pos<player->dmf2_info.dmf2_index_acc) {
			if(!b_avi_load_std_index(player, video_stream, false)) {
					player->dmf2_info.dmf2_index_acc -= video_stream->stdindex.nEntriesInUse;
					continue;
			}
		}

		if(!player->dmf2_indx) {
			rc = bfile_cache_seek(player->idx_cache, index_pos*B_AVI_OLD_INDEX_ENTRY);
		} else {
			rc = bfile_cache_seek(player->idx_cache, (index_pos-player->dmf2_info.dmf2_index_acc)*B_AVI_STD_INDEX_ENTRY);
		}

		if(rc<0) {
			BDBG_WRN(("b_avi_player_next_backward:< %#lx can't access index %u", (unsigned long)player, index_pos));
			break;
		}

		buf = bfile_cache_next(player->idx_cache);
		if(!buf) {
		    BDBG_MSG(("b_avi_player_next_backward: %#lx can't read index %u %s", (unsigned long)player, index_pos, bfile_cache_is_file_error(player->idx_cache)?"file_error":""));
            if(bfile_cache_is_file_error(player->idx_cache)) {
			    rc=-1;
            } else {
                entry->type = bmedia_player_entry_type_end_of_stream;
            }
			break;
		}
		if(player->dmf2_indx) {
			stream=video_stream;
		} else {
			BDBG_MSG_TRACE(("b_avi_player_next_backward: %#lx %u idx:" BMEDIA_FOURCC_FORMAT, (unsigned long)player, (unsigned)index_pos, BMEDIA_FOURCC_ARG(B_AVI_GET_DWORD(buf,0))));
			stream = b_avi_player_get_stream(player, B_AVI_GET_STREAM_ID(buf));
			if(!stream || B_AVI_IS_DRM_HEADER(buf)) {
				continue;
			}
		}

		if(stream==video_stream) {
			unsigned frame_size;
			if(stream->index_entry.pos!=index_pos) {
				if(stream->index_entry.frame_no>0) {
					stream->index_entry.frame_no--;
				} else {
					BDBG_WRN(("b_avi_player_next_backward: %p stream:%u underflow of frame counter", (void *)player, stream->stream_no));
				}
				frame_size = (player->dmf2_indx?B_AVI_GET_STD_OBJ_SIZE(buf):B_AVI_GET_OBJ_SIZE(buf));
				if(stream->index_entry.size >= frame_size) {
					stream->index_entry.size -= frame_size;
				} else {
					BDBG_WRN(("b_avi_player_next_backward: %p stream:%u underflow of frame size %u:%u", (void *)player, stream->stream_no, (unsigned)stream->index_entry.size, frame_size));
					stream->index_entry.size = 0;
				}
			}
			stream->index_entry.pos = index_pos;
			stream->seek_completed = false;
			time = b_avi_player_stream_time(stream);
			BDBG_MSG_TRACE(("b_avi_player_next_backward: %p stream:%u (%u:%u) time:%u(%u) pos:%u", (void *)player, stream->stream_no, stream->index_entry.frame_no, (unsigned)stream->index_entry.size, (unsigned)time, (unsigned)dest_time, index_pos));
			if(time<dest_time && (player->dmf2_indx?B_AVI_IS_STD_KEYFRAME(buf):B_AVI_IS_KEYFRAME(buf))) {
				bavi_off_t start;
				if(player->dmf2_indx) {
					start = B_AVI_GET_STD_CHUNK_OFF(buf) + stream->stdindex.qwBaseOffset;
					if(!b_avi_seek_riff(player, entry, start, index_pos)) {
						BDBG_MSG(("b_avi_player_next_backward: %#lx end of dmf2 segment", (unsigned long)player));
						return 0;
					}
				} else {
					start = B_AVI_GET_CHUNK_OFF(buf);
				}

				/* Always assign video stream to index 0*/
				player->resync.resync[0].stream_id = stream->stream_no;
				player->resync.resync[0].timestamp = time;
				entry->entry = &player->resync;
				entry->timestamp = time;
				entry->start = start+player->index_movi_off;
				entry->length = (player->dmf2_indx?B_AVI_GET_STD_CHUNK_SIZE(buf):B_AVI_GET_CHUNK_SIZE(buf));
				if(player->encrypted_video) {
					if (!player->dmf2_indx) {
						entry->start -= B_AVI_DRM_HEADER_SIZE;
						entry->length += B_AVI_DRM_HEADER_SIZE;
					} else {
						buf = bfile_cache_next(player->idx_cache);
						if(!buf) {
							BDBG_MSG(("b_avi_player_next_backward: %#lx can't read index for frame %u", (unsigned long)player, index_pos));
							rc=-1;
							break;
						}
						start = B_AVI_GET_STD_CHUNK_OFF(buf) + stream->stdindex.qwBaseOffset;
						entry->length = (start-entry->start);
					}
				}

				BDBG_MSG_TRACE(("b_avi_player_next_backward: %#lx found frame(%u:%u) %u:%u(%u)", (unsigned long)player, stream->index_entry.frame_no, (unsigned)time, (unsigned)entry->start, (unsigned)entry->length, (unsigned)index_pos));
				if(index_pos>0) {
					index_pos--;
				}
				rc = 0;
				break;
			}
		}
		if (index_pos==0) {
			BDBG_MSG(("b_avi_player_next_backward: %#lx EOF %u", (unsigned long)player, index_pos));
			rc=-1;
			break;
		}
	}
	player->pos = index_pos;
	return  rc;
}

int
bavi_player_next(bavi_player_t player, bmedia_player_entry *entry)
{
	bfile_io_read_t  fd;
	int rc;

	BDBG_OBJECT_ASSERT(player, bavi_player_t);
	BDBG_MSG_TRACE(("bavi_player_next:> %#lx %u %d %u", (unsigned long)player, (unsigned)player->pos, (int)player->step, (unsigned)player->player_state));
    fd = player->fd;
    bmedia_player_init_entry(entry);

    if(player->eof_reached) {
	    entry->type = bmedia_player_entry_type_end_of_stream;
        goto done;
    }
	entry->type = bmedia_player_entry_type_file;
	/* send a haader first */
	switch(player->player_state) {
	case b_avi_player_state_data:
		break;
	case b_avi_player_state_header:
		entry->start =  0 ;
		entry->length = player->movi_off;
		entry->embedded = NULL;
        entry->content =  bmedia_player_entry_content_header;
		player->player_state = b_avi_player_state_preframe;
		if(player->dmf2_indx) {
			player->player_state = b_avi_player_state_avix_header;
		}
		goto done;
	case b_avi_player_state_avix_header:
		player->player_state = b_avi_player_state_data/* b_avi_player_state_preframe */; /*GM : For avix header we cannot send first video frame from first riff chunk*/
		if(player->dmf2_info.curr_dmf2_riff_indx) {
			entry->start = player->dmf2_info.riff_info[player->dmf2_info.curr_dmf2_riff_indx].start;
			entry->length =  player->dmf2_info.riff_info[player->dmf2_info.curr_dmf2_riff_indx].movi_off;
			entry->embedded = NULL;
			entry->content = bmedia_player_entry_content_header;
			goto done;
		}
		break;
	case b_avi_player_state_preframe:
		player->player_state = b_avi_player_state_data;
		/* If we are not playing from the first frame, always send the first video frame to bavi_stream */
		if(player->first_vframe_off >= 0 && player->preframe_required) {
			b_avi_player_stream  *stream = b_avi_player_get_video_stream(player);
			
			BKNI_Memset(&player->resync, 0, sizeof(bmedia_player_sync_entry));
			player->resync.resync[0].stream_id = stream->stream_no;
			player->resync.resync[0].timestamp = BMEDIA_PLAYER_INVALID;
			entry->entry = &player->resync;
			
			entry->start = player->first_vframe_off;
			entry->length =  player->first_vframe_size;
			entry->embedded = NULL;
			entry->content = bmedia_player_entry_content_payload;
			entry->timestamp = BMEDIA_PLAYER_INVALID;
			player->preframe_required = false;
			player->need_timestamp = true;
		}
        else {
            entry->type = bmedia_player_entry_type_noop;
        }
		goto done;
	case b_avi_player_state_dmf2_segment:
		if((player->dmf2_info.segment.end - player->dmf2_info.segment.start) > DMF2_SEGMENT_SIZE){
			entry->start = player->dmf2_info.segment.start;
			entry->length = DMF2_SEGMENT_SIZE;
			player->dmf2_info.segment.start += DMF2_SEGMENT_SIZE;
			player->player_state = b_avi_player_state_dmf2_segment;
		} else {
			entry->start = player->dmf2_info.segment.start;
			entry->length = player->dmf2_info.segment.end - player->dmf2_info.segment.start;
			player->dmf2_info.segment.start = player->dmf2_info.segment.end = 0;
			player->player_state = b_avi_player_state_data;
		}
        entry->content = bmedia_player_entry_content_header;
		entry->embedded = NULL;
		goto done;
	case b_avi_player_state_embedded:
		entry->start =  player->last_read_data_offset;
		entry->length = player->embedded_length;
		entry->embedded = player->embedded_data;
		entry->type = bmedia_player_entry_type_embedded;
        entry->content = bmedia_player_entry_content_payload;
		player->player_state = b_avi_player_state_data;
		goto done;
	}
	/* now send a data payload */
	entry->content = bmedia_player_entry_content_payload;
	if (player->step == 0) {
		rc = b_avi_player_next_play(player, entry);
	} else if(player->step > 0) {
		rc = b_avi_player_next_forward(player, entry);
	} else { /* player->step < 0 */
		rc = b_avi_player_next_backward(player, entry);
	} 

	if(player->dmf2_indx) {
        if(rc<0) {
            /* for now treat all errors in DMF2 stream as end of file */
            entry->type = bmedia_player_entry_type_end_of_stream;
            rc = 0;
        }
    }

    if(rc==0) {
        if(entry->type == bmedia_player_entry_type_end_of_stream && !player->eof_reached) {
            player->eof_reached = true;
            if(player->dmf2_indx) {
                player->dmf2_info.dmf2_start_offset = player->movi_off;
            }
            entry->length = 1;
            entry->start = player->movi_off + player->movi_size - entry->length;
            entry->type = bmedia_player_entry_type_embedded;
            entry->embedded = player->zero;
            entry->content = bmedia_player_entry_content_header;
            rc = 0;
        } else {
            player->eof_reached = false;
        }
    }


	if(rc<0 || player->eof_reached) {
		return rc;
	}

	/* test if we reached non readable offset */
	if (((off_t) (entry->start + entry->length) > (off_t)player->last_read_data_offset) && (!player->dmf2_indx)) {
		size_t read_length;
		ssize_t read_result;
		off_t read_offset;
		off_t seek_result;

		if( (off_t)(entry->start + entry->length) > (off_t)(player->last_read_data_offset + sizeof(player->embedded_data))) {
		  if((off_t)(entry->start+sizeof(player->embedded_data))<(off_t)player->last_read_data_offset) {
		    entry->length = (player->last_read_data_offset + sizeof(player->embedded_data)) - entry->start;  /* fix read size */
		  } else {
		    entry->start = player->last_read_data_offset;
		    entry->length = sizeof(player->embedded_data);
		  }
		}

		if (entry->start < (off_t)player->last_read_data_offset) {
			read_length = (entry->start + entry->length) - player->last_read_data_offset;
			entry->length = player->last_read_data_offset - entry->start;
			read_offset = player->last_read_data_offset;
			player->player_state = b_avi_player_state_embedded; /* this will cause embedded data to be passed on the next bavi_player_next call */
		} else {
			read_length = entry->length;
			read_offset = entry->start;
			entry->embedded = player->embedded_data;
			entry->type = bmedia_player_entry_type_embedded;
		}
        entry->content = bmedia_player_entry_content_payload;
		BDBG_ASSERT(read_length <= sizeof(player->embedded_data));
		seek_result = fd->seek(fd, read_offset, SEEK_SET);
		if (seek_result != read_offset) {
			BDBG_ERR(("bavi_player_next:%#lx error seeking to the data %lu %lu", (unsigned long)player, (unsigned long)seek_result, (unsigned long)read_offset));
			return BERR_TRACE(-1);
		}
		read_result = fd->read(fd, player->embedded_data, read_length);
		if ((size_t) read_result!=read_length) {
			BDBG_ERR(("bavi_player_next:%#lx error reaing the data %ld %ld", (unsigned long)player, (long)read_result, (long)read_length));
			return BERR_TRACE(-1);
		}
		player->embedded_length = read_length;
	}
done:
	BDBG_MSG(("bavi_player_next: %#lx %u (%lu:%u) %s", (unsigned long)player, (unsigned)player->pos, (unsigned long)entry->start, (unsigned)entry->length, entry->embedded?"embedded":""));
	BDBG_MSG_TRACE(("bavi_player_next:< %#lx %u (%lu:%u:%lu) %s", (unsigned long)player, (unsigned)player->pos, (unsigned long)entry->start, (unsigned)entry->length, (unsigned long)entry->timestamp, entry->embedded?"embedded":""));
	return 0;
}

void
bavi_player_tell(bavi_player_t player, bmedia_player_pos *pos)
{
	b_avi_player_stream *stream;
	BDBG_OBJECT_ASSERT(player, bavi_player_t);

	stream = b_avi_player_get_video_stream(player);
	if(stream) {
	    *pos = b_avi_player_stream_time(stream);
    } else {
		*pos = BMEDIA_PLAYER_INVALID;
	}
	return;
}

void
bavi_player_get_status(bavi_player_t player, bmedia_player_status *status)
{
	BDBG_OBJECT_ASSERT(player, bavi_player_t);
	BDBG_MSG_TRACE(("bavi_player_get_status:> %#lx", (unsigned long)player));
	status->bounds = player->bounds;
	status->bounds.last--;
    status->index_error_cnt = player->index_error_cnt;
	status->direction = player->step;
    bavi_player_tell(player, &status->position);
	BDBG_MSG_TRACE(("bavi_player_get_status:< %#lx %ld:%ld %ld", (unsigned long)player, (long)player->bounds.first, (long)player->bounds.last, (long)player->step));
	return;
}

void
bavi_player_set_direction(bavi_player_t player, bmedia_player_step direction)
{
	BDBG_OBJECT_ASSERT(player, bavi_player_t);
	BDBG_MSG_TRACE(("bavi_player_set_direction:> %#lx %ld", (unsigned long)player, (long)direction));
    player->eof_reached = false;
	player->step = direction;
	player->need_timestamp = true;
	player->player_state = b_avi_player_state_header; /* set state to header in order to resent the ASF header */
	BDBG_MSG_TRACE(("bavi_player_set_direction:< %#lx %ld", (unsigned long)player, (long)player->step));
	return;
}



static bmedia_player_pos
b_avi_player_stream_seek(bavi_player_t player, b_avi_player_stream *stream, bmedia_player_pos pos)
{
	bmedia_player_pos time;
	unsigned index_pos;
	int rc;
	unsigned last_index_pos;
    unsigned duration;
    bmedia_player_pos target_pos = pos;
    bmedia_player_pos prev_seekable_point = player->bounds.first;
    bmedia_player_pos last_seekable_point = player->bounds.first;

    player->eof_reached = false;

    BSTD_UNUSED(last_index_pos);

	for(last_index_pos=(unsigned)-1;;) {
		time  = bmedia_index_get(stream->index, pos, &stream->index_entry);
		if(time==BMEDIA_PLAYER_INVALID) {
			b_avi_index_entry_clear(&stream->index_entry);
			index_pos = 0;
		} else {
			index_pos = stream->index_entry.pos;
		}
		if(stream->status.stream_type==bavi_stream_type_audio) {
			bavi_audio_state_set_memo(&stream->audio, &stream->index_entry.audio_memo, stream->index_entry.frame_no, stream->index_entry.size);
		}

        BDBG_MSG(("b_avi_player_stream_seek: %p stream:%u pos:%u index:%u(%u) %u:%u:%u", (void *)player, stream->stream_no, (unsigned)pos, (unsigned)time, (unsigned)b_avi_player_stream_time(stream), (unsigned)stream->index_entry.frame_no, (unsigned)stream->index_entry.size, (unsigned)stream->index_entry.pos));
        last_index_pos = index_pos;
        rc = bfile_cache_seek(player->idx_cache, index_pos*B_AVI_OLD_INDEX_ENTRY);
        if(rc<0) {
            BDBG_WRN(("b_avi_player_stream_seek:< %p can't access index %u", (void *)player, index_pos));
            return BERR_TRACE(-1);
        }
        if(pos==player->bounds.first) {
            return pos;
        }
        for(;;index_pos++) {
            const uint8_t *buf = bfile_cache_next(player->idx_cache);
            bool is_seekable_point;
            if(!buf) {
                if(bfile_cache_is_file_error(player->idx_cache)) {
                    BDBG_WRN(("b_avi_player_stream_seek: %p can't read index %u %s", (void *)player, index_pos, bfile_cache_is_file_error(player->idx_cache)?"file_error":""));
                    return BMEDIA_PLAYER_INVALID;
                }
                BDBG_MSG(("b_avi_player_stream_seek: %p can't read index %u %s", (void *)player, index_pos, bfile_cache_is_file_error(player->idx_cache)?"file_error":""));
                if (stream->status.stream_type != bavi_stream_type_video){
                    duration = ((unsigned)(((uint64_t)1000*(uint64_t)stream->status.header.dwScale*(uint64_t)stream->status.header.dwLength)/stream->status.header.dwRate));
                    if ((time + B_AVI_AUDIO_INDEX_THRESHOLD) > duration)
                    {
                        BDBG_WRN(("b_avi_player_stream_seek: %p time(%u) was within %ums of track duration",(void *)player, (unsigned)time, B_AVI_AUDIO_INDEX_THRESHOLD));
                        return 0;
                    }
                }
                break; /* return to outer loop and try seek with different offset */
            }
            if((stream->stream_no != B_AVI_GET_STREAM_ID(buf)) || B_AVI_IS_DRM_HEADER(buf)) {
                continue;
            }
            BDBG_MSG_TRACE(("b_avi_player_stream_seek: %p %u idx:" BMEDIA_FOURCC_FORMAT, (void *)player, (unsigned)index_pos, BMEDIA_FOURCC_ARG(B_AVI_GET_DWORD(buf,0))));
            time = b_avi_player_stream_time(stream);
            is_seekable_point = ((stream->status.stream_type != bavi_stream_type_video) || (player->dmf2_indx?B_AVI_IS_STD_KEYFRAME(buf):B_AVI_IS_KEYFRAME(buf)) || pos==player->bounds.first);
            BDBG_MSG_TRACE(("b_avi_player_stream_seek: %p stream:%u (%u:%u) time:%u(%u) pos:%u %s", (void *)player, stream->stream_no, stream->index_entry.frame_no, (unsigned)stream->index_entry.size, (unsigned)time, (unsigned)pos, index_pos, is_seekable_point?"KEY":""));
            stream->index_entry.pos = index_pos;
            if(is_seekable_point) {
                b_avi_player_index_point(player, stream, time, buf);
                last_seekable_point = time;
                if(time>=pos) {
                    BDBG_MSG(("b_avi_player_stream_seek: %p stream:%u (%u:%u) time:%u(%u) pos:%u done", (void *)player, stream->stream_no, stream->index_entry.frame_no, (unsigned)stream->index_entry.size, (unsigned)time, (unsigned)pos, index_pos));
                    return time;
                }
            }
            b_avi_player_update_audio_stream(player, stream, buf);
            b_avi_player_update_stream(player, stream, buf);
        }

		if (prev_seekable_point == last_seekable_point) {
            pos = player->bounds.first;
        } else {
            pos = last_seekable_point;
            prev_seekable_point = last_seekable_point;
        }
		BDBG_MSG(("b_avi_player_stream_seek: %#lx stream %u seek to new location %u(%u)", (unsigned long)player, stream->stream_no, (unsigned)pos, (unsigned)target_pos));
	}
}


static bmedia_player_pos
b_avi_player_stream_seek_dmf2(bavi_player_t player, b_avi_player_stream *stream, bmedia_player_pos pos, int id)
{
	bfile_io_read_t fd;
	off_t cur_pos;
	const uint8_t *buf;
	unsigned index_pos=0;
	bmedia_player_pos time=0;
	bavi_off_t offset=0;	
	int rc;

	BSTD_UNUSED(pos);		

	fd = player->fd;
	cur_pos = fd->seek(fd, 0, SEEK_CUR);

	stream->cur_superindex_entry = -1;
	rc = b_avi_load_std_index(player, stream, true);
	if(rc<0) {
		return BMEDIA_PLAYER_INVALID;
	}

	for(;;index_pos++) {
		buf = bfile_cache_next(player->dmf2_info.dmf2_idx_cache[id]);

		if(!buf) {
			if(b_avi_load_std_index(player, stream, true)==0) {
				continue;
			}
			BDBG_MSG(("b_avi_player_stream_seek: %#lx can't read index %u", (unsigned long)player, index_pos));
			break;
		}

		b_avi_player_update_audio_stream(player, stream, buf);

		offset = ((bavi_off_t)B_AVI_GET_STD_CHUNK_OFF(buf) + stream->stdindex.qwBaseOffset);
		if(offset >= player->dmf2_info.dmf2_start_offset) {

			break;
		}

		stream->index_entry.pos = index_pos;
		stream->index_entry.frame_no++;
		stream->index_entry.size += B_AVI_GET_STD_OBJ_SIZE(buf);
	}

	time = b_avi_player_stream_time(stream);

	stream->index_entry.pos = 0;
	stream->index_entry.frame_no = 0;
	stream->index_entry.size = 0;

	bfile_cache_clear(player->dmf2_info.dmf2_idx_cache[id]);

	fd->seek(fd, cur_pos, SEEK_SET);

	return time;
}


static int
b_avi_player_try_seek(bavi_player_t player, bmedia_player_pos pos)
{
	b_avi_player_stream *stream;
	unsigned index_pos;
	bmedia_player_pos time;
	int rc;
	unsigned streams_to_sync;
	bmedia_player_pos video_time;
	const uint8_t *buf;

	BDBG_OBJECT_ASSERT(player, bavi_player_t);
	BDBG_MSG_TRACE(("b_avi_player_try_seek:> %#lx %ld", (unsigned long)player, (long)pos));
    player->eof_reached = false;
    for(video_time=pos,streams_to_sync=0,index_pos=0,stream=BLST_SQ_FIRST(&player->streams);stream;stream=BLST_SQ_NEXT(stream, link)) {
        time = bmedia_index_get(stream->index, pos, &stream->index_entry);
        if(time==BMEDIA_PLAYER_INVALID) {
            b_avi_index_entry_clear(&stream->index_entry);
            index_pos = 0;
        } else {
            if(streams_to_sync==0) { /* very first stream to sync */
                index_pos = stream->index_entry.pos;
            } else if (stream->index_entry.pos<index_pos) { /* find smallest index entry for all streams */
                index_pos = stream->index_entry.pos;
            }
        }
        if(stream->status.stream_type==bavi_stream_type_audio) {
            bavi_audio_state_set_memo(&stream->audio, &stream->index_entry.audio_memo, stream->index_entry.frame_no, stream->index_entry.size);
        } else if(stream->status.stream_type == bavi_stream_type_video) {
            video_time = BMEDIA_PLAYER_INVALID;
        }
        streams_to_sync++;
        stream->seek_completed = false;
        BDBG_MSG_TRACE(("b_avi_player_try_seek: %p stream:%u pos:%u index:%u(%u) %u:%u:%u:%u", (void *)player, stream->stream_no, (unsigned)pos, (unsigned)time, (unsigned)b_avi_player_stream_time(stream), (unsigned)stream->index_entry.frame_no, (unsigned)stream->index_entry.size, (unsigned)stream->index_entry.pos, (unsigned)index_pos));
    }
	if(player->dmf2_indx) {
		/* For DMF 2 index, we will only seek the video stream*/
		streams_to_sync = 1;
		/* b_avi_seek_std_index(player, pos); */
		stream = b_avi_player_get_video_stream(player);
		b_avi_seek_std_index(player, (unsigned)stream->index_entry.pos);
		index_pos+=player->dmf2_info.dmf2_index_acc;
	}
	BDBG_ASSERT(streams_to_sync>0); /* open_index ensures that there are at least one valid stream */
	BDBG_MSG(("b_avi_player_try_seek: %#lx seeking %u streams from index %u", (unsigned long)player, streams_to_sync, index_pos));

	if(!player->dmf2_indx) {
		rc = bfile_cache_seek(player->idx_cache, index_pos*B_AVI_OLD_INDEX_ENTRY);
	} else {
		rc = bfile_cache_seek(player->idx_cache, (index_pos-player->dmf2_info.dmf2_index_acc)*B_AVI_STD_INDEX_ENTRY);
	}

	if(rc<0) {
		BDBG_WRN(("b_avi_player_try_seek:< %#lx can't access index %u", (unsigned long)player, index_pos));
		return BERR_TRACE(-1);
	}
	for(;streams_to_sync>0;index_pos++) {

		buf = bfile_cache_next(player->idx_cache);
		if(buf) {

			if(player->dmf2_indx) {
				stream = b_avi_player_get_video_stream(player);
			} else {
				unsigned stream_no;
				BDBG_MSG_TRACE(("b_avi_player_try_seek: %#lx %u idx:" BMEDIA_FOURCC_FORMAT, (unsigned long)player, (unsigned)index_pos, BMEDIA_FOURCC_ARG(B_AVI_GET_DWORD(buf,0))));
				stream_no = B_AVI_GET_STREAM_ID(buf);
				stream = b_avi_player_get_stream(player, stream_no);
			}

			if(!stream || stream->seek_completed || stream->index_entry.pos>index_pos || B_AVI_IS_DRM_HEADER(buf)) {
				continue;
			}
			stream->index_entry.pos = index_pos;
			b_avi_player_update_audio_stream(player, stream, buf);
			time = b_avi_player_stream_time(stream);
			BDBG_MSG_TRACE(("b_avi_player_try_seek: %p stream:%u (%u:%u) time:%u(%u) pos:%u", (void *)player, stream->stream_no, stream->index_entry.frame_no, (unsigned)stream->index_entry.size, (unsigned)time, (unsigned)pos, index_pos));

			b_avi_player_index_point(player, stream, time, buf);
			switch(stream->status.stream_type) {
			case bavi_stream_type_video:
            {
                bool key_frame = (player->dmf2_indx?B_AVI_IS_STD_KEYFRAME(buf):B_AVI_IS_KEYFRAME(buf));

				if(time>=pos && key_frame) {
					BDBG_MSG(("b_avi_player_try_seek: %#lx stream:%u video seek completed at %u:%u %u %u", (unsigned long)player, stream->stream_no, (unsigned)time, (unsigned)pos, stream->index_entry.pos, stream->index_entry.frame_no ));
					video_time = time;
					stream->seek_completed = true;
					streams_to_sync--;

					if(player->dmf2_indx) {
						b_avi_player_stream *sync_stream;
						int cnt;

						if(time==0) {
							player->dmf2_info.dmf2_start_offset = player->movi_off;
							for(cnt=0,sync_stream=BLST_SQ_FIRST(&player->streams);sync_stream;sync_stream=BLST_SQ_NEXT(sync_stream, link)) {
							if(sync_stream->status.stream_type == bavi_stream_type_audio)
								player->dmf2_info.dmf2_resync_offset[cnt++] = 0;
							}
						}
						else {
							player->dmf2_info.dmf2_start_offset = B_AVI_GET_STD_CHUNK_OFF(buf) + stream->stdindex.qwBaseOffset;
							for(cnt=0,sync_stream=BLST_SQ_FIRST(&player->streams);sync_stream;sync_stream=BLST_SQ_NEXT(sync_stream, link)) {
								if(sync_stream->status.stream_type == bavi_stream_type_audio) {
									player->dmf2_info.dmf2_resync_offset[cnt] = b_avi_player_stream_seek_dmf2(player, sync_stream, time, cnt) - time;
									cnt++;
									break;
								}
							}
						}
					}
				}
				break;
            }
			case bavi_stream_type_audio:
				if(time>=video_time) {
					BDBG_MSG(("b_avi_player_try_seek: %#lx stream:%u audio seek completed at %u:%u %u", (unsigned long)player, stream->stream_no, (unsigned)time, (unsigned)video_time, stream->index_entry.pos));
					if(video_time!=BMEDIA_PLAYER_INVALID && !bmedia_player_pos_in_range(time,video_time,33)) { /* if audio-video sync larger than 33ms */
						time = b_avi_player_stream_seek(player, stream, video_time);
						BDBG_MSG(("b_avi_player_try_seek: %#lx stream:%u audio seek tuned at %u:%u %u", (unsigned long)player, stream->stream_no, (unsigned)time, (unsigned)video_time, stream->index_entry.pos));
						if(time==BMEDIA_PLAYER_INVALID) {
							return BERR_TRACE(-1);
						}
						rc = bfile_cache_seek(player->idx_cache, (index_pos+1)*B_AVI_OLD_INDEX_ENTRY);
						if(rc<0) {
							BDBG_WRN(("b_avi_player_try_seek:< %#lx can't access index %u", (unsigned long)player, index_pos+1));
							return BERR_TRACE(-1);
						}
					}
					stream->seek_completed = true;
					streams_to_sync--;
				}
				break;
			default:
				break;
			}
			if(!stream->seek_completed) {
				b_avi_player_update_stream(player, stream, buf);
			}
		} else { /* buf == NULL, reached end of stream and didn't find stream entry */
			BDBG_MSG(("b_avi_player_try_seek: %#lx can't read index %u", (unsigned long)player, index_pos));

			if(player->dmf2_indx) {
				b_avi_player_stream *video_stream = b_avi_player_get_video_stream(player);
				if(!b_avi_load_std_index(player, video_stream, true)) {
					player->dmf2_info.dmf2_index_acc = index_pos--;
					continue;
				}
			}else{
				if(video_time==BMEDIA_PLAYER_INVALID) {
					stream = b_avi_player_get_video_stream(player);
					BDBG_ASSERT(stream); /* if video_time==BMEDIA_PLAYER_INVALID then there is should be a video stream */
					video_time = b_avi_player_stream_seek(player, stream, pos);
					if(video_time==BMEDIA_PLAYER_INVALID) {
						return BERR_TRACE(-1);
					}
					stream->seek_completed = true;
				}
				for(stream=BLST_SQ_FIRST(&player->streams);stream;stream=BLST_SQ_NEXT(stream, link)) {
					bmedia_player_pos seek_time;
					if(stream->seek_completed) {
						continue;
					}
					BDBG_MSG(("b_avi_player_try_seek: %#lx seeking stream %u to %u", (unsigned long)player, stream->stream_no, (unsigned)video_time));
					seek_time = b_avi_player_stream_seek(player, stream, video_time);
					if(seek_time==BMEDIA_PLAYER_INVALID) {
						return BERR_TRACE(-1);
					}
				}
			}
			break;
		}
	}

	if(!player->dmf2_indx) {
		/* find earliest enry in the stream */
		stream=BLST_SQ_FIRST(&player->streams);
		BDBG_ASSERT(stream); /* open_index ensures that there are at least one valid stream */
		index_pos = stream->index_entry.pos;
		while(NULL!=(stream=BLST_SQ_NEXT(stream, link))) {
			if(stream->index_entry.pos<index_pos) {
				index_pos = stream->index_entry.pos;
			}
		}
	}
	else {
		stream = b_avi_player_get_video_stream(player);
		index_pos = stream->index_entry.pos;
	}

	if(!player->dmf2_indx) {
		rc = bfile_cache_seek(player->idx_cache, index_pos*B_AVI_OLD_INDEX_ENTRY);
	}
	else {
		rc = bfile_cache_seek(player->idx_cache, (index_pos-player->dmf2_info.dmf2_index_acc)*B_AVI_STD_INDEX_ENTRY);
	}

	if(rc<0) {
		BDBG_WRN(("b_avi_player_try_seek:< %#lx can't access index %u", (unsigned long)player, index_pos));
		return BERR_TRACE(-1);
	}
	player->need_timestamp = true;
	player->pos = index_pos;
	player->player_state = b_avi_player_state_header; /* set state to header in order to resent the ASF header */
    player->preframe_required = pos>0; BDBG_MSG_TRACE(("b_avi_player_try_seek:< %#lx %u:%u:%u", (unsigned long)player, (unsigned)video_time,(unsigned)pos, index_pos));
	return 0;
}

int
bavi_player_seek(bavi_player_t player, bmedia_player_pos pos)
{
    int rc;

    if (pos >= player->bounds.last || pos < player->bounds.first) {
        BDBG_WRN(("bavi_player_seek:< %p out of bounds %u<%u>%u %ld", (void *)player, (unsigned)player->bounds.first, (unsigned)pos, (unsigned)player->bounds.last, (long)-1));
        pos = player->bounds.first;
    }
    rc = b_avi_player_try_seek(player, pos);
    return rc;
}

static void *
b_avi_player_create(bfile_io_read_t fd, const bmedia_player_config *config, const bmedia_player_stream *stream)
{
	return bavi_player_create(fd, config, stream);
}

static void
b_avi_player_destroy(void *player)
{
	bavi_player_destroy(player);
}

static int
b_avi_player_next(void *player, bmedia_player_entry *entry)
{
	return bavi_player_next(player, entry);
}

static void
b_avi_player_tell(void *player, bmedia_player_pos *pos)
{
	bavi_player_tell(player, pos);
	return;
}

static void
b_avi_player_get_status(void *player, bmedia_player_status *status)
{
	bavi_player_get_status(player, status);
	return;
}

static int
b_avi_player_set_direction(void *player, bmedia_player_step direction, bmedia_time_scale time_scale, bmedia_player_decoder_mode *mode)
{
	BSTD_UNUSED(time_scale);
    BSTD_UNUSED(mode);
	bavi_player_set_direction(player, direction);
	return 0;
}

static int
b_avi_player_seek(void *player, bmedia_player_pos pos)
{
	return bavi_player_seek(player, pos);
}

const bmedia_player_methods bavi_player_methods = {
	b_avi_player_create,
	b_avi_player_destroy,
	b_avi_player_next,
	b_avi_player_tell,
	b_avi_player_get_status,
	b_avi_player_set_direction,
	b_avi_player_seek
};


