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
 * BMedia library, stream probe module
 *
 *******************************************************************************/
#include "bstd.h"
#include "bavi_probe.h"
#include "bavi_parser.h"
#include "bavc_video_probe.h"
#include "bmedia_player.h"
#include "bkni.h"

BDBG_MODULE(bavi_probe);

typedef struct bavi_probe  *bavi_probe_t; 

typedef struct b_avi_probe_object_handler {
	bavi_parser_handler handler; /* must be first */
	bavi_probe_t probe; /* pointer to probe */ 
} b_avi_probe_object_handler;

typedef struct b_avi_index_track_info {
    bavi_probe_track *track;
    bmedia_player_pos cur_time;
    off_t file_offset;
    unsigned stream_len;
    unsigned frame_no;
    bavi_audio_state audio_state;
    bavi_streamheader header;
    batom_cursor cursor;
} b_avi_index_track_info;


struct bavi_probe {
	BDBG_OBJECT(bavi_probe_t)
	bavi_parser_t parser;
	bavi_probe_stream *stream;
	bavi_probe_track *track;
	bavi_off_t movi_off;
	bavi_dword movi_size;
	bavi_off_t index_off;
	bavi_dword index_size;
	bool track_header_valid;
	bool track_stream_info_valid;
	bool finished;
	bool stream_error;
    bool dmlh_found;
    bool mainheader_found;
    bool rec_list_detected;
	unsigned track_no;
	b_avi_probe_object_handler hdrl_handler;
	b_avi_probe_object_handler strl_handler;
	b_avi_probe_object_handler odml_handler;
	b_avi_probe_object_handler rec_handler;
	b_avi_probe_object_handler vchunk_handler[2];
	bavi_mainheader mainheader;
    bavi_probe_track* h264_track;

    b_avi_index_track_info audio_stream_index[BMEDIA_PLAYER_MAX_TRACKS];
    b_avi_index_track_info video_track;
    unsigned num_audio;
    bavi_streamheader header;
    bmedia_waveformatex waveForm;
    bool dmf2_indx;
};

#define B_AVI_GET_STREAM_ID(p)	(1+(10*(unsigned)(((uint8_t*)(p))[0]-'0') + (((uint8_t*)(p))[1]-'0')))
#define B_AVI_OLD_INDEX_ENTRY ( 4 /* dwChunkId */ + 4 /* dwFlags */ + 4 /* dwOffset */ + 4 /* dwSize */)

static void
b_avi_probe_parse_index(bavi_probe_t probe, bfile_buffer_t buf)
{
    off_t gap;
    uint32_t i;
    batom_t atom;
    bfile_buffer_result result;
    uint32_t dwChunkId, dwFlags, dwOffset, dwSize;


    if (probe->dmf2_indx || !probe->index_off || !probe->index_size || !probe->video_track.track) {
        return;
    }

    atom = bfile_buffer_read(buf, probe->index_off, probe->index_size, &result);
    if (!atom) {
        BDBG_ERR(("b_avi_probe_parse_index: %p can't read entire contents of index (offset:%u size:%u) in a single chunk. Multiple reads not yet implemented.", (void *)probe, (unsigned )probe->index_off, probe->index_size));
        return;
    }

    batom_cursor_from_atom(&probe->video_track.cursor, atom);
    for(i=0;i<probe->num_audio && i<BMEDIA_PLAYER_MAX_TRACKS;i++) {
        batom_cursor_from_atom(&probe->audio_stream_index[i].cursor,atom);
    }

    while(batom_cursor_size(&probe->video_track.cursor) >= B_AVI_OLD_INDEX_ENTRY) {
        dwChunkId = batom_cursor_uint32_le(&probe->video_track.cursor);
        dwFlags = batom_cursor_uint32_le(&probe->video_track.cursor);
        dwOffset = batom_cursor_uint32_le(&probe->video_track.cursor);
        dwSize = batom_cursor_uint32_le(&probe->video_track.cursor);
        BSTD_UNUSED(dwFlags);

        if (probe->video_track.track->media.number != B_AVI_GET_STREAM_ID(&dwChunkId)) {
            continue;
        }

        probe->video_track.cur_time = (uint32_t)(((probe->video_track.header.dwStart+probe->video_track.frame_no) * ((uint64_t)probe->video_track.header.dwScale*1000)/probe->video_track.header.dwRate));
        probe->video_track.file_offset = dwOffset;
	    probe->video_track.frame_no++;

        for(i=0;i<probe->num_audio;i++) {
            /* Default to the video offset.  This will be updated if there is a valid audio index */
            probe->audio_stream_index[i].file_offset = probe->video_track.file_offset; 

            while((probe->audio_stream_index[i].cur_time < probe->video_track.cur_time) && (batom_cursor_size(&probe->audio_stream_index[i].cursor) >= B_AVI_OLD_INDEX_ENTRY)){
                dwChunkId = batom_cursor_uint32_le(&probe->audio_stream_index[i].cursor);
                dwFlags = batom_cursor_uint32_le(&probe->audio_stream_index[i].cursor);
                dwOffset = batom_cursor_uint32_le(&probe->audio_stream_index[i].cursor);
                dwSize = batom_cursor_uint32_le(&probe->audio_stream_index[i].cursor);

                if (probe->audio_stream_index[i].track->media.number != B_AVI_GET_STREAM_ID(&dwChunkId)) {
                    continue;
                }

                bavi_audio_state_update(&probe->audio_stream_index[i].audio_state, dwSize);
                probe->audio_stream_index[i].cur_time = bavi_audio_state_get_timestamp(&probe->audio_stream_index[i].audio_state, probe->audio_stream_index[i].frame_no, probe->audio_stream_index[i].stream_len);
                probe->audio_stream_index[i].stream_len += dwSize;
                probe->audio_stream_index[i].file_offset = dwOffset;
                probe->audio_stream_index[i].frame_no++;
            }
            gap = (probe->video_track.file_offset>probe->audio_stream_index[i].file_offset) ? (off_t)(probe->video_track.file_offset-probe->audio_stream_index[i].file_offset):(off_t)(probe->audio_stream_index[i].file_offset-probe->video_track.file_offset);

            if (gap > probe->stream->max_av_byte_delta) {
                BDBG_MSG(("Video time %ums, setting max gap to " B_OFFT_FMT, (unsigned)probe->video_track.cur_time, B_OFFT_ARG(gap)));
                probe->stream->max_av_byte_delta = gap;
            }
        }
    }

    if(atom) {
        batom_release(atom);
    }

    return;
}


static bavi_parser_action b_avi_probe_vpayload(bavi_parser_handler *handler, bavi_fourcc fourcc, batom_t object);

BDBG_OBJECT_ID(bavi_probe_t);


static bool 
b_avi_probe_header_match(batom_cursor *header)
{
	bavi_fourcc riff;
	bavi_fourcc avi;
	
	riff = bavi_read_fourcc(header);
	batom_cursor_skip(header, sizeof(bavi_atom) /* size */);
	avi = bavi_read_fourcc(header);
	if(BATOM_IS_EOF(header)) {
		return false;
	}
	return riff==BMEDIA_FOURCC('R','I','F','F') && avi==BMEDIA_FOURCC('A','V','I', ' ');
}


static const bmedia_probe_file_ext b_avi_ext[] =  {
	{"avi"},{"divx"},{"xvid"},
	{""}
};

static bavi_parser_action  
b_avi_probe_hdrl(bavi_parser_handler *handler, bavi_fourcc fourcc, batom_t object)
{
	bavi_probe_t probe = ((b_avi_probe_object_handler*)handler)->probe;
	bavi_probe_stream *stream;

	BDBG_MSG(("b_avi_probe_hdrl: %p " BMEDIA_FOURCC_FORMAT "[" BMEDIA_FOURCC_FORMAT "] %u bytes", (void *)probe, BMEDIA_FOURCC_ARG(handler->fourcc), BMEDIA_FOURCC_ARG(fourcc), object?(unsigned)batom_len(object):0));
	BDBG_OBJECT_ASSERT(probe, bavi_probe_t);
	switch(fourcc) {
	case BAVI_FOURCC_BEGIN:
		stream = BKNI_Malloc(sizeof(*stream));
		if(!stream) {
			BDBG_ERR(("b_avi_probe_hdrl: %p can't allocate %u bytes", (void *)probe, (unsigned)sizeof(*stream)));
			probe->stream_error = true;
			return bavi_parser_action_return;
		}
		bmedia_probe_stream_init(&stream->media, bstream_mpeg_type_avi);
        stream->video_framerate = 0;
        stream->suggestedBufferSize = 0;
        stream->max_av_byte_delta = -1;
        probe->dmf2_indx = false;
		probe->stream = stream;
		break;
	case BMEDIA_FOURCC('a','v','i','h'):
		if(!bavi_read_mainheader(&probe->mainheader, object)) {
			BDBG_WRN(("b_avi_probe_hdrl: %#lx error in the main header", (unsigned long)probe));
			goto done;
		}
        probe->mainheader_found = true;
		if(probe->stream) {
			probe->stream->media.duration = ((uint64_t)probe->mainheader.dwMicroSecPerFrame * probe->mainheader.dwTotalFrames)/1000;
			probe->stream->media.max_bitrate = probe->mainheader.dwMaxBytesPerSec*8;
            if(probe->mainheader.dwMicroSecPerFrame > 0) {
                probe->stream->video_framerate = 1000000000/probe->mainheader.dwMicroSecPerFrame;
            }
            probe->stream->suggestedBufferSize = probe->mainheader.dwSuggestedBufferSize;
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
b_avi_probe_strl(bavi_parser_handler *handler, bavi_fourcc fourcc, batom_t object)
{
	bavi_probe_t probe = ((b_avi_probe_object_handler*)handler)->probe;
	bavi_probe_track *track;
	bmedia_bitmapinfo video;
	batom_cursor c;
	size_t length;
	uint8_t ch;
	unsigned i;
	static const char audio_str[] = "Audio - ";
	char type[8];

	BDBG_OBJECT_ASSERT(probe, bavi_probe_t);
	BDBG_MSG(("b_avi_probe_strl: " BMEDIA_FOURCC_FORMAT "[" BMEDIA_FOURCC_FORMAT "] %u bytes", BMEDIA_FOURCC_ARG(handler->fourcc), BMEDIA_FOURCC_ARG(fourcc), object?(unsigned)batom_len(object):0));
	track = probe->track;
	switch(fourcc) {
	case BAVI_FOURCC_BEGIN:
		BDBG_ASSERT(track==NULL);
		track = BKNI_Malloc(sizeof(*track));
		if(!track) {
			BDBG_ERR(("b_avi_probe_strl: %p can't allocate %u bytes", (void *)track, (unsigned)sizeof(*track)));
			goto done;
		}
		bmedia_probe_track_init(&track->media);
		probe->track_no++;
		track->media.number = probe->track_no;
		track->private_data = NULL;
        track->private_data_length = 0;
        track->duration = 0;
		track->encryptedContentFlag = false;
		track->language[0] = '\0';
		probe->track_header_valid = false;
		probe->track_stream_info_valid = false;
        track->suggestedBufferSize = 0;
        track->averageBitrate = 0;
        track->media_type = 0;
		probe->track = track;
		break;
	case BMEDIA_FOURCC('s','t','r','h'):
		BDBG_ASSERT(object);
		if(!track) {
			goto done;
		}
		probe->track_header_valid = bavi_read_streamheader(&probe->header, object);
		if(!probe->track_header_valid) {
			goto done;
		}
		switch(probe->header.fccType) {
		case BMEDIA_FOURCC('a','u','d','s'):
			track->media.type = bmedia_track_type_audio;
			break;
		case BMEDIA_FOURCC('v','i','d','s'):
			track->media.type = bmedia_track_type_video;
			break;
		default:
			track->media.type = bmedia_track_type_other;
			probe->track_stream_info_valid = true;
			break;
		}
        if(probe->header.dwRate) {
            track->duration = (unsigned)(((uint64_t)1000*(uint64_t)probe->header.dwScale*(uint64_t)probe->header.dwLength)/probe->header.dwRate);
        }
        track->suggestedBufferSize = probe->header.dwSuggestedBufferSize;
		break;
	case BMEDIA_FOURCC('s','t','r','f'):
		BDBG_ASSERT(object);
		if(!track) {
			goto done;
		}
		if(!probe->track_header_valid) {
			goto done;
		}
		switch(track->media.type) {
        case bmedia_track_type_audio:
            batom_cursor_from_atom(&c,object);

            probe->track_stream_info_valid = bmedia_read_waveformatex(&probe->waveForm, &c);
            if(!probe->track_stream_info_valid) {
                goto done;
            }
            track->media_type = probe->waveForm.wFormatTag;
            if(BMEDIA_WAVFMTEX_AUDIO_PCM(&probe->waveForm)) {
                track->media.info.audio.codec = baudio_format_pcm;
            } else if(BMEDIA_WAVFMTEX_AUDIO_WMA(&probe->waveForm)) {
                track->media.info.audio.codec = baudio_format_wma_std;
            } else if(BMEDIA_WAVFMTEX_AUDIO_WMA_PRO(&probe->waveForm)) {
                track->media.info.audio.codec = baudio_format_wma_pro;
            } else if(BMEDIA_WAVFMTEX_AUDIO_MPEG(&probe->waveForm)) {
                track->media.info.audio.codec = baudio_format_mpeg;
            } else if(BMEDIA_WAVFMTEX_AUDIO_MP3(&probe->waveForm)) {
                track->media.info.audio.codec = baudio_format_mp3;
            } else if(BMEDIA_WAVFMTEX_AUDIO_AC3(&probe->waveForm)) {
                track->media.info.audio.codec = baudio_format_ac3;
            } else if(BMEDIA_WAVFMTEX_AUDIO_DTS(&probe->waveForm)) {
                track->media.info.audio.codec = baudio_format_dts;
            } else if(BMEDIA_WAVFMTEX_AUDIO_G726(&probe->waveForm)) {
                track->media.info.audio.codec = baudio_format_g726;
            } else if(BMEDIA_WAVFMTEX_AUDIO_MS_ADPCM(&probe->waveForm)) {
                track->media.info.audio.codec = baudio_format_adpcm;
            } else if(BMEDIA_WAVFMTEX_AUDIO_DVI_ADPCM(&probe->waveForm)) {
                track->media.info.audio.codec = baudio_format_dvi_adpcm;
            } else if(BMEDIA_WAVFMTEX_AUDIO_AAC(&probe->waveForm)) {
                track->media.info.audio.codec = baudio_format_aac;
            }  else {
                track->media.info.audio.codec = baudio_format_unknown;
            }
            track->media.info.audio.channel_count = probe->waveForm.nChannels;
            track->media.info.audio.sample_size = probe->waveForm.wBitsPerSample;
            track->media.info.audio.sample_rate = probe->waveForm.nSamplesPerSec;
            track->media.info.audio.bitrate = (probe->waveForm.nAvgBytesPerSec*8)/1000;
            track->averageBitrate = track->media.info.audio.bitrate;
            break;
		case bmedia_track_type_video:
			batom_cursor_from_atom(&c,object);
			probe->track_stream_info_valid = bmedia_read_bitmapinfo(&video, &c);
			if(!probe->track_stream_info_valid) {
				goto done;
			}
            track->media_type = video.biCompression;
            track->media.info.video.timestamp_order = bmedia_timestamp_order_decode;
			if(BMEDIA_FOURCC_DIVX5_CODEC(video.biCompression) || BMEDIA_FOURCC_XVID_CODEC(video.biCompression) || BMEDIA_FOURCC_3IVX_CODEC(video.biCompression) ) {
				track->media.info.video.codec = bvideo_codec_mpeg4_part2;
			} else if(BMEDIA_FOURCC_DIVX3_CODEC(video.biCompression)) {
				track->media.info.video.codec = bvideo_codec_divx_311;
			} else if(BMEDIA_FOURCC_VC1SM_CODEC(video.biCompression)) {
				track->media.info.video.codec = bvideo_codec_vc1_sm;
			} else if(BMEDIA_FOURCC_VC1AP_CODEC(video.biCompression)) {
				track->media.info.video.codec = bvideo_codec_vc1;
			} else if(BMEDIA_FOURCC_MJPEG_CODEC(video.biCompression)) {
				track->media.info.video.codec = bvideo_codec_mjpeg;
			} else if(BMEDIA_FOURCC_H264_CODEC(video.biCompression)) {
				track->media.info.video.codec = bvideo_codec_h264;
                if(probe->h264_track==NULL) {
                    char strm_no_0, strm_no_1;
                    probe->h264_track = track;
                    strm_no_0 = '0' | ((track->media.number-1)/10)%10;
                    strm_no_1 = '0' | (track->media.number-1)%10;
                    bavi_parser_install_handler(probe->parser, &probe->vchunk_handler[0].handler, BMEDIA_FOURCC(strm_no_0,strm_no_1,'d','c'), b_avi_probe_vpayload);
                    bavi_parser_install_handler(probe->parser, &probe->vchunk_handler[1].handler, BMEDIA_FOURCC(strm_no_0,strm_no_1,'d','b'), b_avi_probe_vpayload);
                }
			} else {
				track->media.info.video.codec = bvideo_codec_unknown;
			}
			track->media.info.video.width = video.biWidth;
			track->media.info.video.height = video.biHeight;
			
			switch(video.biCompression) {
			case BMEDIA_FOURCC('D','X','S','A'):
				track->subtitle_type = bavi_subtitle_type_dxsa;
				break;
			case BMEDIA_FOURCC('D','X','S','B'):
				track->subtitle_type = bavi_subtitle_type_dxsb;
				break;
			default:
				track->subtitle_type = bavi_subtitle_type_none;
				break;
			}

			break;
		default:
			break;
		}
		break;
	case BMEDIA_FOURCC('s','t','r','d'):		
		BDBG_ASSERT(object);
		if(!track) {
			goto done;
		}
		if(!probe->track_header_valid) {
			goto done;
		}
		if(!probe->track_stream_info_valid){
			goto done;
		}
		length = batom_len(object);
		track->private_data = BKNI_Malloc(length);
		track->private_data_length = length;
		batom_cursor_from_atom(&c, object);		
		batom_cursor_copy(&c, track->private_data, track->private_data_length);
		track->encryptedContentFlag = true;
		break;
	case BMEDIA_FOURCC('s','t','r','n'):
		BDBG_ASSERT(object);
		if(!track) {
			goto done;
		}
		batom_cursor_from_atom(&c, object);
		batom_cursor_copy(&c, type, sizeof(type));
		if(BATOM_IS_EOF(&c)) {
			break;
		}
		if(BKNI_Memcmp(type, audio_str, sizeof(type))) {
			break;
		}
		for(i = 0; i < sizeof(track->language)-1 && (ch = batom_cursor_byte(&c)) != ';'; i++) {
			if(BATOM_IS_EOF(&c)) {
				i = 0;
				break;
			}
			track->language[i] = ch;            
		}
		track->language[i] = '\0';
		break;
    case BMEDIA_FOURCC('i','n','d','x'):
        probe->dmf2_indx=true;
        /* If a rec list is detected the index will not be usable. Only validate the index if there is no rec list */
        if(probe->stream && !probe->rec_list_detected) {
            probe->stream->media.index = bmedia_probe_index_available;
        }
	    break;
	case BAVI_FOURCC_END:
		if(!track || !probe->stream) {
			goto done;
		}
		probe->track = NULL;
		if(!probe->track_header_valid || !probe->track_stream_info_valid) {
			BKNI_Free(track);
			goto done;
		}

        if(track->media.type == bmedia_track_type_audio) {
            probe->audio_stream_index[probe->num_audio].track = track;
            probe->audio_stream_index[probe->num_audio].header = probe->header;
            bavi_audio_state_init(&probe->audio_stream_index[probe->num_audio].audio_state);
            bavi_audio_state_set_header(&probe->audio_stream_index[probe->num_audio].audio_state, &probe->header, &probe->waveForm);
            probe->num_audio++;
        } else if ((track->media.type == bmedia_track_type_video) && (((bavi_probe_track *)track)->subtitle_type == bavi_subtitle_type_none)) {
            probe->video_track.track = track;
            probe->video_track.header = probe->header;
        }
        /* coverity[address_free] */
        bmedia_probe_add_track(&probe->stream->media, &track->media);  /* free is not bad since &track->media == track */

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
b_avi_probe_odml(bavi_parser_handler *handler, bavi_fourcc fourcc, batom_t object)
{
	bavi_probe_t probe = ((b_avi_probe_object_handler*)handler)->probe;
	bavi_dmlh dmlh;

	BDBG_MSG(("b_avi_probe_odml: %p " BMEDIA_FOURCC_FORMAT "[" BMEDIA_FOURCC_FORMAT "] %u bytes", (void *)probe, BMEDIA_FOURCC_ARG(handler->fourcc), BMEDIA_FOURCC_ARG(fourcc), object?(unsigned)batom_len(object):0));
	BDBG_OBJECT_ASSERT(probe, bavi_probe_t);
	switch(fourcc) {
	case BAVI_FOURCC_BEGIN:
        break;
	case BMEDIA_FOURCC('d','m','l','h'):
		if(!bavi_read_dmlh(&dmlh, object)) {
			BDBG_WRN(("b_avi_probe_odml: %#lx error in the odml header", (unsigned long)probe));
			goto done;
		}
        probe->dmlh_found = true;
		if(probe->stream && probe->mainheader_found) {
			probe->stream->media.duration = ((uint64_t)probe->mainheader.dwMicroSecPerFrame * dmlh.dwTotalFrames)/1000;
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
b_avi_probe_rec(bavi_parser_handler *handler, bavi_fourcc fourcc, batom_t object)
{
	bavi_probe_t probe = ((b_avi_probe_object_handler*)handler)->probe;

	BDBG_MSG(("b_avi_probe_rec: %p " BMEDIA_FOURCC_FORMAT "[" BMEDIA_FOURCC_FORMAT "] %u bytes", (void *)probe, BMEDIA_FOURCC_ARG(handler->fourcc), BMEDIA_FOURCC_ARG(fourcc), object?(unsigned)batom_len(object):0));

    if(fourcc==BAVI_FOURCC_BEGIN) {
        /* Currently we don't handle the rec list, mark the index as usable. */
        probe->stream->media.index = bmedia_probe_index_missing;
        probe->rec_list_detected = true;
    }

	if(object) {
		batom_release(object);
	}
	return bavi_parser_action_return;
}

static bavi_parser_action 
b_avi_probe_vpayload(bavi_parser_handler *handler, bavi_fourcc fourcc, batom_t object)
{
	bavi_probe_t probe = ((b_avi_probe_object_handler*)handler)->probe;
	BSTD_UNUSED(fourcc);
	if (object && probe->h264_track) {
        bmedia_probe_track* track = &probe->h264_track->media;
		batom_cursor cursor;
		uint32_t scode;

        BDBG_ASSERT(track->type==bmedia_track_type_video && track->info.video.codec==bvideo_codec_h264);

		batom_cursor_from_atom(&cursor, object);
		while ((scode = bmedia_video_scan_scode(&cursor, 0xFFFFFFFFul))) {
			if ((scode&0x80)==0x0 && /* forbidden_zero_bit */ 
				(scode&0x07)==0x7) { /* nal_unit_type 7 = SPS */
                bmedia_probe_h264_video *h264_info = (bmedia_probe_h264_video *)&track->info.video.codec_specific;
                uint8_t buf[8];

                size_t size = batom_cursor_copy(&cursor, &buf, sizeof(buf));
                b_h264_video_sps_parse(&h264_info->sps, buf, size);
                probe->finished = true;
				break;
			}
		}
	}
	if (object) {
		batom_release(object);
	}
	if(probe->finished) {
        return bavi_parser_action_return;
    }
    return bavi_parser_action_none;
}

static bavi_parser_action 
b_avi_probe_object_begin(void *cntx, bavi_fourcc fourcc, bavi_dword size, bavi_off_t offset)
{
	bavi_probe_t probe = cntx;


	BDBG_OBJECT_ASSERT(probe, bavi_probe_t);
    switch(fourcc) {
    case BMEDIA_FOURCC('m','o','v','i'):
        probe->movi_off = offset;
        probe->movi_size = size;
        /* We're not finished yet, we need to parse the first chunk after the movi object to check for a rec list */
        break;
	case BMEDIA_FOURCC('i','d','x','1'):
		probe->index_size = size - (size%B_AVI_OLD_INDEX_ENTRY);
		probe->index_off = offset;
        /* If a rec list is detected the index will not be usable. Only validate the index if there is no rec list */
		if(size>B_AVI_OLD_INDEX_ENTRY && probe->stream && !probe->rec_list_detected) {
            probe->stream->media.index = bmedia_probe_index_available;
		}
        probe->finished = true;
		break;
    default:
        /* We have found the first chunk after the movi object, if we're not probing further in the ES we're done */
        if(probe->movi_off>=0 && probe->h264_track==NULL) {
		    probe->finished = true;
        }
        break;
    }

	if(probe->finished) {
        return bavi_parser_action_return;
    }
    return bavi_parser_action_none;
}

static bmedia_probe_base_t 
b_avi_probe_create(batom_factory_t factory)
{
	bavi_probe_t	probe;
	bavi_parser_cfg cfg;

	probe = BKNI_Malloc(sizeof(*probe));
	if(!probe) {
		BDBG_ERR(("b_avi_probe_create: can't allocate %u bytes", (unsigned)sizeof(*probe)));
		goto err_alloc;
	}
	BDBG_OBJECT_INIT(probe, bavi_probe_t);
	probe->stream = NULL;
	probe->hdrl_handler.probe = probe;
	probe->strl_handler.probe = probe;
	probe->odml_handler.probe = probe;
    probe->rec_handler.probe = probe;
	probe->vchunk_handler[0].probe = probe;
	probe->vchunk_handler[1].probe = probe;
    probe->h264_track = NULL;
	bavi_parser_default_cfg(&cfg);
    cfg.user_cntx = probe;
    cfg.object_begin = b_avi_probe_object_begin;
	probe->parser = bavi_parser_create(factory, &cfg);
	if(!probe->parser) {
		goto err_parser;
	}
	bavi_parser_install_handler(probe->parser, &probe->hdrl_handler.handler, BMEDIA_FOURCC('h','d','r','l'), b_avi_probe_hdrl); 
	bavi_parser_install_handler(probe->parser, &probe->strl_handler.handler, BMEDIA_FOURCC('s','t','r','l'), b_avi_probe_strl); 
	bavi_parser_install_handler(probe->parser, &probe->odml_handler.handler, BMEDIA_FOURCC('o','d','m','l'), b_avi_probe_odml); 
    bavi_parser_install_handler(probe->parser, &probe->rec_handler.handler, BMEDIA_FOURCC('r','e','c',' '), b_avi_probe_rec);
	probe->vchunk_handler[0].handler.handler = 0;
	probe->vchunk_handler[1].handler.handler = 0;

    BKNI_Memset(&probe->video_track, 0, sizeof(probe->video_track));
    BKNI_Memset(&probe->audio_stream_index, 0, sizeof(probe->audio_stream_index));
    probe->num_audio = 0;

	return (bmedia_probe_base_t)probe;
err_parser:
	BKNI_Free(probe);
err_alloc:
	return NULL;
}

static void 
b_avi_probe_destroy(bmedia_probe_base_t probe_)
{
	bavi_probe_t probe = (bavi_probe_t) probe_;

	BDBG_OBJECT_ASSERT(probe, bavi_probe_t);
	bavi_parser_remove_handler(probe->parser, &probe->hdrl_handler.handler);
	bavi_parser_remove_handler(probe->parser, &probe->strl_handler.handler);
	bavi_parser_remove_handler(probe->parser, &probe->odml_handler.handler);
	bavi_parser_remove_handler(probe->parser, &probe->rec_handler.handler);
	bavi_parser_destroy(probe->parser);
	BDBG_OBJECT_DESTROY(probe, bavi_probe_t);
	BKNI_Free(probe);
	return;
}

static const bmedia_probe_stream *
b_avi_probe_parse(bmedia_probe_base_t probe_, bfile_buffer_t buf, batom_pipe_t pipe, const bmedia_probe_parser_config *config)
{
    bavi_probe_t probe = (bavi_probe_t)probe_;
    off_t off;
    size_t read_len = BMEDIA_PROBE_FEED_SIZE;
    bavi_probe_stream *stream;

	BDBG_OBJECT_ASSERT(probe, bavi_probe_t);
	BDBG_ASSERT(probe->stream==NULL);
	probe->track = NULL;
	probe->finished = false;
	probe->stream_error = false;
	probe->track_no = 0;
    probe->mainheader_found = false;
    probe->h264_track = NULL;
    probe->movi_off = -1;
    probe->movi_size = 0;
    probe->index_off = 0;
    probe->index_size = 0;
    probe->num_audio = 0;
    probe->dmf2_indx = false;
    probe->rec_list_detected = false;

	for(off=0;off<=256*1024;) {
		batom_t atom;
		bfile_buffer_result result;
		size_t feed_len;
		size_t atom_len;

		BDBG_MSG(("b_avi_probe_parse: %p reading %u:%u", (void *)probe, (unsigned)(off+config->parse_offset), (unsigned)read_len));
		atom = bfile_buffer_read(buf, off+config->parse_offset, read_len, &result);
		if(!atom) {
			break;
		}
		atom_len = batom_len(atom);
		BDBG_MSG(("b_avi_probe_parse: %p read %u:%u -> %#lx", (void *)probe, (unsigned)(off+config->parse_offset), (unsigned)atom_len, (unsigned long)atom));
		off += atom_len;
		batom_pipe_push(pipe, atom);
		feed_len = bavi_parser_feed(probe->parser, pipe);
		if(feed_len!=atom_len || probe->stream_error) {
			break;
		}
        if(probe->finished) {
            if(probe->stream && config->probe_index && probe->stream->media.index == bmedia_probe_index_unknown) {
                probe->stream->media.index = bmedia_probe_index_missing;
                if(probe->movi_size) {
                    unsigned i;
                    batom_pipe_flush(pipe);
                    off = probe->movi_off + probe->movi_size;
                    bavi_parser_seek(probe->parser, off);
                    probe->finished = true;
                    for(i=0; i< (16 * 4096) && probe->stream->media.index != bmedia_probe_index_available;) {
                        BDBG_MSG(("b_avi_probe_parse: %p reading index %u:%u", (void *)probe, (unsigned)(off+config->parse_offset), (unsigned)read_len));
                        atom = bfile_buffer_read(buf, off+config->parse_offset, read_len, &result);
                        if(!atom) {
                            break;
                        }
                        atom_len = batom_len(atom);
                        BDBG_MSG(("b_avi_probe_parse: %p read index %u:%u -> %p", (void *)probe, (unsigned)(off+config->parse_offset), (unsigned)atom_len, (void *)atom));
                        off += atom_len;
                        i += atom_len;
                        batom_pipe_push(pipe, atom);
                        feed_len = bavi_parser_feed(probe->parser, pipe);
                        if(feed_len!=atom_len || probe->stream_error) {
                            break;
                        }
                    }
                }
            }

            if (probe->stream && config->parse_index) {
                b_avi_probe_parse_index(probe, buf);
            }
            break;
        }
    }
    if (probe->h264_track) {
        bavi_parser_remove_handler(probe->parser, &probe->vchunk_handler[0].handler);
        bavi_parser_remove_handler(probe->parser, &probe->vchunk_handler[1].handler);
    }
    bavi_parser_reset(probe->parser);
    /* return result of parsing */
    stream = probe->stream;
    if(stream) {
        const bmedia_probe_track *track;
        bool valid_video=false;
        for(track=BLST_SQ_FIRST(&stream->media.tracks);track;track=BLST_SQ_NEXT(track, link)) {
            if(track->type == bmedia_track_type_video && track->info.video.codec != bvideo_codec_unknown) {
                valid_video = true;
                break;
            }
        }
        if(!valid_video && stream->media.index == bmedia_probe_index_available) {
            stream->media.index = bmedia_probe_index_unusable;
        }
    }
    probe->stream = NULL;
    return &stream->media;
}

static void
b_avi_probe_stream_free(bmedia_probe_base_t probe_, bmedia_probe_stream *stream)
{
	bavi_probe_t probe = (bavi_probe_t)probe_;
	bavi_probe_track *track;
	BDBG_OBJECT_ASSERT(probe, bavi_probe_t);
	BSTD_UNUSED(probe);

	BDBG_ASSERT(stream);

	while(NULL!=(track=(bavi_probe_track*)BLST_SQ_FIRST(&stream->tracks))) {
		BLST_SQ_REMOVE_HEAD(&stream->tracks, link);
		if(track->private_data){
		  BKNI_Free(track->private_data);
		}
		BKNI_Free(track);
	}
	BKNI_Free(stream);
	return;
}


const bmedia_probe_format_desc bavi_probe = {
	bstream_mpeg_type_avi,
	b_avi_ext, /* ext_list */
	/* RIFF 			   size				AVI */
	sizeof(bavi_fourcc)+sizeof(bavi_atom)+sizeof(bavi_fourcc), /* header_size */
	b_avi_probe_header_match, /* header_match */
	b_avi_probe_create, /* create */
	b_avi_probe_destroy, /* destroy */
	b_avi_probe_parse, /* parse */
	b_avi_probe_stream_free /* stream free */
};

