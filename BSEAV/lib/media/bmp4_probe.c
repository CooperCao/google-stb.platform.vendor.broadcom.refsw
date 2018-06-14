/***************************************************************************
 * Copyright (C) 2007-2018 Broadcom.  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
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
#include "bmp4_probe.h"
#include "bmp4_parser.h"
#include "bkni.h"
#include "bmedia_util.h"
#include "bavc_video_probe.h"
#include "baac_probe.h"
#include "bfile_cache.h"

BDBG_MODULE(bmp4_probe);

typedef struct bmp4_probe  *bmp4_probe_t; 

typedef struct b_mp4_probe_handler {
	bmp4_parser_handler handler; /* must be first */
	bmp4_probe_t probe; /* pointer to probe */ 
} b_mp4_probe_handler;

struct bmp4_probe {
	BDBG_OBJECT(bmp4_probe_t)
	bmp4_parser_t parser;
	bmp4_probe_stream *stream;
	bmp4_probe_track *track;
	off_t next_seek;
	bool movieheader_valid;
	bool movie_valid;
	bool stream_error;
	bool trackheader_valid;
	bool sample_valid;
	bool sample_size_valid;
	bool mediaheader_valid;
	bool handler_valid;
    bool mdat_valid;
    bfile_segment samplesize;
    bfile_segment compactsamplesize;
    batom_factory_t factory;
    uint32_t handler_type;
	b_mp4_probe_handler filetype_handler;
	b_mp4_probe_handler movie_handler;
	b_mp4_probe_handler track_handler;
	b_mp4_probe_handler media_handler;
	b_mp4_probe_handler mediainfo_handler;
	b_mp4_probe_handler sampletable_handler;
};

BDBG_OBJECT_ID(bmp4_probe_t);

static bool 
b_mp4_probe_header_match(batom_cursor *header)
{
	bmp4_box box;
	size_t header_size;

	header_size = bmp4_parse_box(header, &box);
	if(header_size==0) {
		return false;
	}
    switch(box.type) {
    default:
        return false;

        /* QuickTime File Format Specification */
    case BMP4_TYPE('w','i','d','e'):
    case BMP4_TYPE('f','r','e','e'):
    case BMP4_TYPE('s','k','i','p'):

        /* ISO/IEC 14496-12:2005 MPEG-4 Part 12 - ISO Base Media File Format */
        /* page 5 */
    case BMP4_FILETYPEBOX:
    case BMP4_MOVIE:
    case BMP4_MOVIE_DATA:
        return true;
    }
}

static const bmedia_probe_file_ext b_mp4_ext[] =  {
	{"mp4"},
	{""}
};

static bmp4_probe_stream *
b_mp4_get_stream(bmp4_probe_t probe)
{
    bmp4_probe_stream *stream;

    stream = probe->stream;
    if(!stream) {
        stream = BKNI_Malloc(sizeof(*stream));
        if(stream) {
            bmedia_probe_stream_init(&stream->media, bstream_mpeg_type_mp4);
            stream->media.index = bmedia_probe_index_required;
            stream->mdat.offset = 0;
            stream->mdat.size = 0;
            stream->moov.offset = 0;
            stream->moov.size = 0;
            stream->mvex.offset = 0;
            stream->mvex.size = 0;
            stream->rmra.offset = 0;
            stream->rmra.size = 0;
            stream->uuid.offset = 0;
            stream->uuid.size = 0;
            stream->compatible = false;
            stream->fragmented = false;
            stream->ftyp[0] = '\0';
            probe->stream = stream;
        } else {
            BDBG_ERR(("b_mp4_probe_hdrl: %p can't allocate %u bytes", (void *)probe, (unsigned)sizeof(*stream)));
        }
    }
    return stream;
}

static bmp4_parser_action  
b_mp4_probe_filetype(bmp4_parser_handler *handler, uint32_t type, batom_t box)
{
	bmp4_probe_t probe = ((b_mp4_probe_handler *)handler)->probe;
    static const uint32_t 
    b_mp4_compatible_brands[] = 
    {
        BMP4_TYPE('q','t',' ',' '),
        BMP4_TYPE('3','g','2','a'),
        BMP4_TYPE('i','s','o','m'),
        BMP4_TYPE('i','s','o','2'),
        BMP4_TYPE('m','p','4','1'),
        BMP4_TYPE('m','p','4','2')
    };


	BDBG_MSG(("b_mp4_player_filetype:%p " B_MP4_TYPE_FORMAT "[" B_MP4_TYPE_FORMAT "] %u bytes", (void *)probe, B_MP4_TYPE_ARG(handler->type), B_MP4_TYPE_ARG(type), box?(unsigned)batom_len(box):0));
	BDBG_OBJECT_ASSERT(probe, bmp4_probe_t);
	if(type == BMP4_FILETYPEBOX) {
        bmp4_filetypebox filetype;
		BDBG_ASSERT(box);
		if(bmp4_parse_filetype(box, &filetype)) {
            bmp4_probe_stream *stream = b_mp4_get_stream(probe);
            if(stream) {
                unsigned n;
                uint32_t brand = filetype.major_brand;
                stream->ftyp[0] = (brand>>24)&0xFF;
                stream->ftyp[1] = (brand>>16)&0xFF;
                stream->ftyp[2] = (brand>>8)&0xFF;
                stream->ftyp[3] = brand&0xFF;
                stream->ftyp[4] = '\0';
                for(n=0;;n++) {
                    unsigned i;
        		    for(i=0;i<sizeof(b_mp4_compatible_brands)/sizeof(*b_mp4_compatible_brands);i++) {
                        if(brand == b_mp4_compatible_brands[i]) {
                            stream->compatible = true;
                            goto done;
                        }
                    }
                    if(n>=filetype.ncompatible_brands) {
                        break;
                    }
                    brand = filetype.compatible_brands[n];
                }
done:
                ;
			}
        }
	}
	if(box) {
		batom_release(box);
	}
	return bmp4_parser_action_none;
}


#if 0
	brand = batom_cursor_uint32_be(header);
	batom_cursor_skip(header, sizeof(uint32_t) /* version */);
	box_left = box.size - (header_size+sizeof(uint32_t)+sizeof(uint32_t));
	for(;;) {
		unsigned i;

		if(BATOM_IS_EOF(header)) {
			return false;
		}
		BDBG_MSG(("b_mp4_probe_header_match: brand " B_MP4_TYPE_FORMAT , B_MP4_TYPE_ARG(brand)));
		for(i=0;i<sizeof(b_mp4_compatible_brands)/sizeof(*b_mp4_compatible_brands);i++) {
			if(brand == b_mp4_compatible_brands[i]) {
				return true;
			}
		}
		/* read minor brands */
		brand = batom_cursor_uint32_be(header);
		if(box_left<sizeof(uint32_t)) {
			return false;
		}
		box_left -= sizeof(uint32_t);
	} 
#endif

#define BMEDIA_OFFSETOF(type, field) offsetof(type, field)
/* #define BMEDIA_OFFSETOF(type, field) ((uint8_t *)(&((type *)0)->field) - (uint8_t *)0) */ /* this definition generates compiler warnings on 2.6.37 kernel toolchain */

static void
b_mp4_probe_set_segment(bmp4_probe_t probe, unsigned segment_offset, uint64_t size, uint64_t offset)
{
    bmp4_probe_stream *stream;
    stream = b_mp4_get_stream(probe);
    if(stream) {
        bmp4_probe_segment *segment = (void *)((uint8_t *)stream + segment_offset);
        segment->offset = offset;
        segment->size = size;
    }
    return;
}

static bmp4_parser_action
b_mp4_probe_object_begin(void *cntx, uint32_t type, uint64_t size, uint64_t offset, size_t header_size)
{
    bmp4_probe_t probe = cntx;
    bmp4_probe_stream *stream=NULL;
    bmp4_parser_action result = bmp4_parser_action_none;
    unsigned i;
    static const uint32_t skip_boxes [] = {
        BMP4_DECODETIMETOSAMPLE,
        BMP4_COMPOSITIONTIMETOSAMPLE,
        BMP4_SAMPLESIZE,
        BMP4_COMPACTSAMPLESIZE,
        BMP4_SAMPLETOCHINK,
        BMP4_CHUNKOFFSET,
        BMP4_CHUNKLARGEOFFSET,
        BMP4_SYNCSAMPLE,
        BMP4_SAMPLE_AUXILIARY_INFORMATION_SIZES,
        BMP4_SAMPLE_AUXILIARY_INFORMATION_OFFSETS,
        BMP4_SAMPLE_TO_GROUP,
        BMP4_SAMPLE_GROUP_DESCRIPTION,
        BMP4_TYPE('s','e','n','c'),
        BMP4_TYPE('s','t','s','h'),
        BMP4_TYPE('s','t','d','p'),
        BMP4_TYPE('p','a','d','b'),
        BMP4_TYPE('s','d','t','p'),
        BMP4_TYPE('s','b','g','p'),
        BMP4_TYPE('u','u','i','d'),
        BMP4_TYPE('r','m','r','a'),
        BMP4_TYPE('s','u','b','s'),
        BMP4_MOVIE_DATA
    };

    BDBG_MSG(("b_mp4_probe_object_begin:%p " B_MP4_TYPE_FORMAT " %u bytes at %#lx", (void *)cntx, B_MP4_TYPE_ARG(type), (unsigned)size, (unsigned long)offset));
    BSTD_UNUSED(header_size);
    BDBG_OBJECT_ASSERT(probe, bmp4_probe_t);
    switch(type) {
    case BMP4_MOVIE_DATA:
        probe->mdat_valid = true;
        b_mp4_probe_set_segment(probe, BMEDIA_OFFSETOF(bmp4_probe_stream, mdat), size, offset);
        break;
    case BMP4_MOVIE:
        b_mp4_probe_set_segment(probe, BMEDIA_OFFSETOF(bmp4_probe_stream, moov), size, offset);
        break;
    case BMP4_TYPE('r','m','r','a'):
        b_mp4_probe_set_segment(probe, BMEDIA_OFFSETOF(bmp4_probe_stream, rmra), size, offset);
        break;
    case BMP4_TYPE('u','u','i','d'):
        b_mp4_probe_set_segment(probe, BMEDIA_OFFSETOF(bmp4_probe_stream, uuid), size, offset);
        break;
    case BMP4_COMPACTSAMPLESIZE:
        bfile_segment_set(&probe->compactsamplesize, offset, size);
        result = bmp4_parser_action_return;
        break;
    case BMP4_SAMPLESIZE:
        bfile_segment_set(&probe->samplesize, offset, size);
        result = bmp4_parser_action_return;
        break;
    case BMP4_MOVIE_FRAGMENT:
    case BMP4_MOVIE_EXTENDS:
        if(type == BMP4_MOVIE_EXTENDS) {
            b_mp4_probe_set_segment(probe, BMEDIA_OFFSETOF(bmp4_probe_stream, mvex), size, offset);
        }
        stream = b_mp4_get_stream(probe);
        if(stream) {
            stream->fragmented = true;
        }
        result = bmp4_parser_action_return;
        break;
    case BMP4_SAMPLEDESCRIPTION:
        if(probe->track) {
            probe->track->sampledescription.size = size;
            probe->track->sampledescription.offset = offset;
        }
        break;
    case BMP4_MEDIAHEADER:
        if(probe->track) {
            probe->track->mediaheader.size = size;
            probe->track->mediaheader.offset = offset;
        }
        break;
    default:
        break;
    }

    if(size>1024) { /* don't bother with small boxes */
        for(i=0;i<sizeof(skip_boxes)/sizeof(*skip_boxes);i++) {
            if(type==skip_boxes[i]) {
                probe->next_seek = offset+size; /* skip over large boxes */
                return bmp4_parser_action_return;
            }
        }
    }
    return result;
}

static bmp4_parser_action  
b_mp4_probe_movie(bmp4_parser_handler *handler, uint32_t type, batom_t box)
{
	bmp4_probe_t probe = ((b_mp4_probe_handler *)handler)->probe;
	bmp4_parser_action action = bmp4_parser_action_none;
	bmp4_movieheaderbox movieheader;

	BDBG_MSG(("b_mp4_probe_movie:%p " B_MP4_TYPE_FORMAT "[" B_MP4_TYPE_FORMAT "] %u bytes", (void *)probe, B_MP4_TYPE_ARG(handler->type), B_MP4_TYPE_ARG(type), box?(unsigned)batom_len(box):0));
	BDBG_OBJECT_ASSERT(probe, bmp4_probe_t);
	switch(type) {
	case BMP4_TYPE_BEGIN:
		b_mp4_get_stream(probe);
		break;
	case BMP4_MOVIEHEADER:
		BDBG_ASSERT(box);
		probe->movieheader_valid = bmp4_parse_movieheader(box, &movieheader);
		if(!probe->movieheader_valid) {
			BDBG_MSG(("b_mp4_probe_movie:%#lx error in the movie header", (unsigned long)probe));
			goto error;
		}
		if(probe->stream) {
			probe->stream->media.duration = (1000*movieheader.duration)/movieheader.timescale;
		}
		break;
	case BMP4_TYPE_END:
		BDBG_MSG(("b_mp4_probe_movie:%#lx parsing completed", (unsigned long) probe));
		probe->movie_valid = true;
		action = bmp4_parser_action_return;
		break;
	default:
		break;
	}
done:
	if(box) {
		batom_release(box);
	}
	return action;
error:
	probe->stream_error = true;
	action = bmp4_parser_action_return;
	goto done;
}

static bool
b_mp4_probe_validate_sample_rate(bmp4_probe_t probe, bmp4_probe_track *track)
{
    bmedia_info_aac aac_info;

    /* Verify we have a valid sample rate as coded by the stream */
    if (!bmedia_info_aac_set_sampling_frequency_index(&aac_info, track->media.info.audio.sample_rate)) {
        unsigned calc_sample_rate;

        if ( !(probe->sample_size_valid && probe->mediaheader_valid && track->duration && track->sample_count) ) {
    		BDBG_ERR(("%s: Information necessary to calculate the samplerate not available, using an unsupported sample rate %u", "b_mp4_probe_validate_sample_rate", track->media.info.audio.sample_rate));
            return false;
        }

        /* The coded sample rate is not known by our decoder.  Try to calculate it... */
        calc_sample_rate = (1000 /*convert to seconds*/ * 1024 /*convert to Kbytes*/ * (uint64_t)track->sample_count) / track->duration;
        if (bmedia_info_aac_set_sampling_frequency_index(&aac_info, calc_sample_rate)) {
            unsigned sample_rate = bmedia_info_aac_sampling_frequency_from_index(aac_info.sampling_frequency_index);
            /* The calculated sample rate matches our list */
    		BDBG_WRN(("%s: Track sample rate %u is unrecognized. Calculating the rate to be %u", "b_mp4_probe_validate_sample_rate", track->media.info.audio.sample_rate, sample_rate));
            track->media.info.audio.sample_rate = sample_rate;
        } else {
    		BDBG_ERR(("%s: Track using an unsupported sample rate %u", "b_mp4_probe_validate_sample_rate", track->media.info.audio.sample_rate));
            return false;
        }
    } else {
        /* The sample rate was detected in our psupported list. Now, just ensure the value is normalized to be compatible with our decoder (e.g. 48002 will become 48000) */
        track->media.info.audio.sample_rate = bmedia_info_aac_sampling_frequency_from_index(aac_info.sampling_frequency_index);
    }

    /* track->media.info.audio.sample_rate has been verified to contain a known sample rate */
    return true;
}

static bmp4_parser_action  
b_mp4_probe_track(bmp4_parser_handler *handler, uint32_t type, batom_t box)
{
	bmp4_probe_t probe = ((b_mp4_probe_handler *)handler)->probe;
	bmp4_parser_action action = bmp4_parser_action_none;
	bmp4_probe_track *track;
	bmp4_trackheaderbox trackheader;

	BDBG_MSG(("b_mp4_probe_track:%p " B_MP4_TYPE_FORMAT "[" B_MP4_TYPE_FORMAT "] %u bytes", (void *)probe, B_MP4_TYPE_ARG(handler->type), B_MP4_TYPE_ARG(type), box?(unsigned)batom_len(box):0));
	BDBG_OBJECT_ASSERT(probe, bmp4_probe_t);
	track = probe->track;
	switch(type) {
	case BMP4_TRACKHEADER:
		BDBG_ASSERT(box);
		if(!track) {
			break;
		}
		probe->trackheader_valid = bmp4_parse_trackheader(box, &trackheader);
		if(!probe->trackheader_valid) {
			break;
		}
		track->media.number = trackheader.track_ID;
		break;
	case BMP4_TYPE_BEGIN:
		BDBG_MSG(("b_mp4_probe_track:%p creating new track", (void *)probe));
		track = BKNI_Malloc(sizeof(*track));
		if(!track) {
			BDBG_ERR(("b_mp4_probe_track: %p can't allocate %u bytes", (void *)track, (unsigned)sizeof(*track)));
			break;
		}
		bmedia_probe_track_init(&track->media);
        track->sampledescription.offset = 0;
        track->sampledescription.size = 0;
        track->mediaheader.offset = 0;
        track->mediaheader.size = 0;
        track->duration = 0;
        track->sample_count = 0;
        track->encrypted = false;
		probe->trackheader_valid = false;
		probe->sample_valid = false;
		probe->sample_size_valid = false;
		probe->mediaheader_valid = false;
		probe->handler_valid = false;
        bfile_segment_clear(&probe->compactsamplesize);
        bfile_segment_clear(&probe->samplesize);
		probe->track = track;
		break;
	case BMP4_TYPE_END:
		if(!track) {
			break;	
		}
		if(probe->trackheader_valid && probe->sample_valid && probe->mediaheader_valid && probe->handler_valid) {
			BDBG_ASSERT(probe->stream);

            if (track->media.type == bmedia_track_type_audio &&
                track->media.info.audio.codec != baudio_format_unknown &&
                track->media.info.audio.sample_rate != 0 ) {
                b_mp4_probe_validate_sample_rate(probe, track);
            }
            BDBG_MSG(("track: %u sampledescription:%u(%u)", track->media.number, (unsigned)track->sampledescription.offset, (unsigned)track->sampledescription.size));
            /* coverity[address_free] */
            bmedia_probe_add_track(&probe->stream->media, &track->media); /* free is not bad since &track->media == track */
		} else {
			BKNI_Free(track);
		}

        probe->track = NULL;
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
b_mp4_probe_media(bmp4_parser_handler *handler_, uint32_t type, batom_t box)
{
	bmp4_probe_t probe = ((b_mp4_probe_handler *)handler_)->probe;
	bmp4_parser_action action = bmp4_parser_action_none;
	bmp4_probe_track *track;
	bmp4_mediaheaderbox mediaheader;
	bmp4_handlerbox handler;

	BDBG_MSG(("b_mp4_probe_media:%p " B_MP4_TYPE_FORMAT "[" B_MP4_TYPE_FORMAT "] %u bytes", (void *)probe, B_MP4_TYPE_ARG(handler_->type), B_MP4_TYPE_ARG(type), box?(unsigned)batom_len(box):0));
	BDBG_OBJECT_ASSERT(probe, bmp4_probe_t);
	track = probe->track;
	if(track) {
		switch(type) {
		case BMP4_MEDIAHEADER:
			BDBG_ASSERT(box);
			probe->mediaheader_valid = bmp4_parse_mediaheader(box, &mediaheader);
			if(probe->mediaheader_valid) {
               track->duration = (1000*mediaheader.duration)/mediaheader.timescale;
               BDBG_CASSERT(sizeof(track->language)==sizeof(mediaheader.language));
               BKNI_Memcpy(track->language,mediaheader.language, sizeof(track->language));
            } else {
				BDBG_WRN(("b_mp4_probe_media: %#lx track:%u error in parsing Media Header", (unsigned long)probe, track->media.number));
			}
			break;
		case BMP4_HANDLER:
			BDBG_ASSERT(box);
			probe->handler_valid = bmp4_parse_handler(box, &handler);
			if(!probe->handler_valid) {
				BDBG_WRN(("b_mp4_probe_media: %#lx track:%u error in parsing Handler Reference", (unsigned long)probe, track->media.number));
			} 
			probe->handler_type = handler.handler_type;
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
b_mp4_probe_sampletable(bmp4_parser_handler *handler, uint32_t type, batom_t box)
{
    bmp4_probe_t probe = ((b_mp4_probe_handler *)handler)->probe;
    bmp4_parser_action action = bmp4_parser_action_none;
    bmp4_sample_info sample_info;
    bmp4_probe_track *track;

    BDBG_MSG(("b_mp4_probe_sampletable:%p " B_MP4_TYPE_FORMAT "[" B_MP4_TYPE_FORMAT "] %u bytes", (void *)probe, B_MP4_TYPE_ARG(handler->type), B_MP4_TYPE_ARG(type), box?(unsigned)batom_len(box):0));
    BDBG_OBJECT_ASSERT(probe, bmp4_probe_t);
    track = probe->track;
    if(track) {
        switch(type) {
        case BMP4_SAMPLEDESCRIPTION:
            if(probe->handler_valid) {
                probe->sample_valid = bmp4_parse_sample_info(box, &sample_info, probe->handler_type);
                track->encrypted = false;
                if(probe->sample_valid) {
                    if(sample_info.entry_count>0) {
                        unsigned i;
                        const bmp4_audiosampleentry *audio=NULL;
                        const bmp4_visualsampleentry *visual=NULL;
                        const bmpeg4_es_descriptor *audio_mpeg4=NULL;
                        bmp4_sampleentry *sample = sample_info.entries[0];
                        track->encrypted = sample->encrypted;
                        track->protection_scheme_information_size = sample->protection_scheme_information_size;
                        BDBG_CASSERT(sizeof(track->protection_scheme_information)==sizeof(sample->protection_scheme_information));
                        BKNI_Memcpy(track->protection_scheme_information, sample->protection_scheme_information, sizeof(track->protection_scheme_information));
                        switch(sample->sample_type) {
                        case bmp4_sample_type_avc:
                            visual = &sample->codec.avc.visual;
                            track->media.info.video.codec = bvideo_codec_h264;
                            for(i=0;i<sample->codec.avc.meta.sps.no;i++) {
                                bmedia_probe_h264_video *h264_info = (bmedia_probe_h264_video *)&track->media.info.video.codec_specific;
                                size_t data_len;
                                const uint8_t *data;

                                data = bmedia_seek_h264_meta_data(&sample->codec.avc.meta.sps, i, &data_len);
                                if(data==NULL) {
                                    break;
                                }
                                if(data_len==0) { continue;}
                                b_h264_video_sps_parse(&h264_info->sps, data+1, data_len-1);
                                if(h264_info->sps.valid) {
                                    break;
                                }
                            }
                            break;
                        case bmp4_sample_type_mp4v:
                            visual = &sample->codec.mp4v.visual;
                            track->media.info.video.codec = bvideo_codec_mpeg4_part2;
                            break;
                        case bmp4_sample_type_s263:
                            visual = &sample->codec.s263.visual;
                            track->media.info.video.codec = bvideo_codec_h263;
                            break;
                        case bmp4_sample_type_mp4a:
                            track->media.type = bmedia_track_type_audio;
                            track->media.info.audio.channel_count = sample->codec.mp4a.audio.channelcount;
                            track->media.info.audio.sample_size = sample->codec.mp4a.audio.samplesize;
                            if(sample->codec.mp4a.audio.samplerate>0) {
                                track->media.info.audio.sample_rate = sample->codec.mp4a.audio.samplerate>>16;
                            } else if(sample->codec.mp4a.mpeg4.decoder.iso_14496_3.samplingFrequencyIndex!=0x0F) {
                                track->media.info.audio.sample_rate = bmedia_info_aac_sampling_frequency_from_index(sample->codec.mp4a.mpeg4.decoder.iso_14496_3.samplingFrequencyIndex);
                            } else {
                                track->media.info.audio.sample_rate = sample->codec.mp4a.mpeg4.decoder.iso_14496_3.samplingFrequency;
                            }
                            if(sample->codec.mp4a.mpeg4.decoder.iso_14496_3.audioObjectType==5) {
                                track->media.info.audio.codec = baudio_format_aac_plus_adts;
                            } else if(sample->codec.mp4a.mpeg4.decoder.iso_14496_3.audioObjectType==23 || sample->codec.mp4a.mpeg4.decoder.iso_14496_3.audioObjectType==39) {
                                track->media.info.audio.codec = baudio_format_aac_plus_loas;
                            } else {
                                track->media.info.audio.codec = baudio_format_aac;
                            }
                            BDBG_CASSERT(sizeof(bmedia_probe_aac_audio) <= sizeof(track->media.info.audio.codec_specific));
                            if (sample->codec.mp4a.mpeg4.decoder.iso_14496_3.audioObjectType <= 5)
                            {
                                ((bmedia_probe_aac_audio*)&track->media.info.audio.codec_specific)->profile =
                                    (bmedia_probe_aac_profile)sample->codec.mp4a.mpeg4.decoder.iso_14496_3.audioObjectType;
                            }
                            else
                            {
                                ((bmedia_probe_aac_audio*)&track->media.info.audio.codec_specific)->profile =
                                    bmedia_probe_aac_profile_unknown;
                            }
                            break;
                        case bmp4_sample_type_mpg:
                            track->media.info.audio.codec = baudio_format_mpeg;
                            audio = &sample->codec.mp4a.audio;
                            audio_mpeg4 =&sample->codec.mp4a.mpeg4;
                            break;
                        case bmp4_sample_type_als:
                            track->media.info.audio.codec = baudio_format_als;
                            audio = &sample->codec.mp4a.audio;
                            audio_mpeg4 =&sample->codec.mp4a.mpeg4;
                            break;
                        case bmp4_sample_type_ac3:
                        case bmp4_sample_type_eac3:
                            track->media.info.audio.codec = sample->sample_type==bmp4_sample_type_ac3? baudio_format_ac3 : baudio_format_ac3_plus;
                            audio = &sample->codec.ac3.audio;
                            break;
                        case bmp4_sample_type_ac4:
                            track->media.info.audio.codec = baudio_format_ac4;
                            audio = &sample->codec.ac4.audio;
                            break;
                        case bmp4_sample_type_samr:
                        case bmp4_sample_type_sawb:
                        case bmp4_sample_type_sawp:
                            audio = &sample->codec.amr.audio;
                            track->media.info.audio.codec = sample->sample_type==bmp4_sample_type_samr ? baudio_format_amr_nb : baudio_format_amr_wb;
                            break;
                        case bmp4_sample_type_drms:
                            track->media.type = bmedia_track_type_audio;
                            track->media.info.audio.codec = baudio_format_unknown;
                            track->encrypted = true;
                            break;
                        case bmp4_sample_type_drmi:
                            track->media.type = bmedia_track_type_video;
                            track->media.info.video.codec = bvideo_codec_unknown;
                            track->encrypted = true;
                            break;
                        case bmp4_sample_type_qt_ima_adpcm:
                        case bmp4_sample_type_qt_ima4_adpcm:
                            track->media.info.audio.codec = baudio_format_dvi_adpcm;
                            audio = &sample->codec.ms.audio;
                            break;
                        case bmp4_sample_type_mjpeg:
                            visual = &sample->codec.mjpeg.visual;
                            track->media.info.video.codec = bvideo_codec_mjpeg;
                            break;
                        case bmp4_sample_type_twos:
                            track->media.info.audio.codec = baudio_format_pcm;
                            audio = &sample->codec.twos.audio;
                            break;
                        case bmp4_sample_type_dts:
                            audio = &sample->codec.dts.audio;
                            switch(sample->codec.dts.type) {
                            case BMP4_SAMPLE_DTSE:
                                track->media.info.audio.codec = baudio_format_dts_lbr; break;
                            default:
                                track->media.info.audio.codec = baudio_format_dts; break;
                            }
                            break;
                        case bmp4_sample_type_hevc:
                            visual = &sample->codec.hevc.visual;
                            track->media.info.video.codec = bvideo_codec_h265;
                            break;
                        case bmp4_sample_type_mp3:
                            track->media.info.audio.codec = baudio_format_mp3;
                            audio = &sample->codec.ac3.audio;
                            break;
                        case bmp4_sample_type_unknown:
                            track->media.type = bmedia_track_type_other;
                            break;
                        }
                        if(visual) {
                            track->media.type = bmedia_track_type_video;
                            track->media.info.video.width = visual->width;
                            track->media.info.video.height = visual->height;
                        } else if(audio) {
                            track->media.type = bmedia_track_type_audio;
                            track->media.info.audio.channel_count = audio->channelcount;
                            track->media.info.audio.sample_size = audio->samplesize;
                            track->media.info.audio.sample_rate = audio->samplerate>>16;
                            if(track->media.info.audio.sample_rate==0 && audio_mpeg4 && audio_mpeg4->decoder.iso_14496_3.samplingFrequencyIndex==0x0F) {
                                track->media.info.audio.sample_rate = sample->codec.mp4a.mpeg4.decoder.iso_14496_3.samplingFrequency;
                            }
                        }
                    } else {
                        BDBG_WRN(("b_mp4_probe_media: %#lx track:%u Sample Description invalid number of entries", (unsigned long)probe, track->media.number));
                        probe->sample_valid = false;
                    }
                    bmp4_free_sample_info(&sample_info);
                } else {
                    BDBG_WRN(("b_mp4_probe_media: %#lx track:%u error in parsing Sample Description", (unsigned long)probe, track->media.number));
                }
            } else {
                BDBG_WRN(("b_mp4_probe_media: %#lx track:%u Handler Reference invalid, can't parse Sample Description", (unsigned long)probe, track->media.number));
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
b_mp4_probe_mediainfo(bmp4_parser_handler *handler, uint32_t type, batom_t box)
{
    bmp4_probe_t probe = ((b_mp4_probe_handler *)handler)->probe;
    bmp4_parser_action action = bmp4_parser_action_none;

    BSTD_UNUSED(type);

    BDBG_MSG(("b_mp4_probe_mediainfo:%p " B_MP4_TYPE_FORMAT "[" B_MP4_TYPE_FORMAT "] %u bytes", (void *)probe, B_MP4_TYPE_ARG(handler->type), B_MP4_TYPE_ARG(type), box?(unsigned)batom_len(box):0));
    BDBG_OBJECT_ASSERT(probe, bmp4_probe_t);
    if(box) {
        batom_release(box);
    }
    return action;
}


static bmedia_probe_base_t 
b_mp4_probe_create(batom_factory_t factory)
{
	bmp4_probe_t	probe;
	bmp4_parser_cfg cfg;

	probe = BKNI_Malloc(sizeof(*probe));
	if(!probe) {
		BDBG_ERR(("b_mp4_probe_create: can't allocate %u bytes", (unsigned)sizeof(*probe)));
		goto err_alloc;
	}
	BDBG_OBJECT_INIT(probe, bmp4_probe_t);
	probe->stream = NULL;
	probe->filetype_handler.probe = probe;
	probe->movie_handler.probe = probe;
	probe->track_handler.probe = probe;
	probe->media_handler.probe = probe;
	probe->mediainfo_handler.probe = probe;
	probe->sampletable_handler.probe = probe;
    probe->factory = factory;

	bmp4_parser_default_cfg(&cfg);
	cfg.user_cntx = probe;
	cfg.box_begin = b_mp4_probe_object_begin;
	probe->parser = bmp4_parser_create(factory, &cfg);
	if(!probe->parser) {
		goto err_parser;
	}
	bmp4_parser_install_handler(probe->parser, &probe->filetype_handler.handler, BMP4_FILETYPEBOX, b_mp4_probe_filetype); 
	bmp4_parser_install_handler(probe->parser, &probe->movie_handler.handler, BMP4_MOVIE, b_mp4_probe_movie); 
	bmp4_parser_install_handler(probe->parser, &probe->track_handler.handler, BMP4_TRACK, b_mp4_probe_track); 
	bmp4_parser_install_handler(probe->parser, &probe->media_handler.handler, BMP4_MEDIA, b_mp4_probe_media); 
	bmp4_parser_install_handler(probe->parser, &probe->mediainfo_handler.handler, BMP4_MEDIAINFORMATION, b_mp4_probe_mediainfo); 
	bmp4_parser_install_handler(probe->parser, &probe->sampletable_handler.handler, BMP4_SAMPLETABLE, b_mp4_probe_sampletable); 
	return (bmedia_probe_base_t)probe;
err_parser:
	BKNI_Free(probe);
err_alloc:
	return NULL;
}

static void 
b_mp4_probe_destroy(bmedia_probe_base_t probe_)
{
	bmp4_probe_t probe = (bmp4_probe_t)probe_;

	BDBG_OBJECT_ASSERT(probe, bmp4_probe_t);
	bmp4_parser_remove_handler(probe->parser, &probe->sampletable_handler.handler);
	bmp4_parser_remove_handler(probe->parser, &probe->mediainfo_handler.handler);
	bmp4_parser_remove_handler(probe->parser, &probe->media_handler.handler);
	bmp4_parser_remove_handler(probe->parser, &probe->track_handler.handler);
	bmp4_parser_remove_handler(probe->parser, &probe->movie_handler.handler);
	bmp4_parser_remove_handler(probe->parser, &probe->filetype_handler.handler);
	bmp4_parser_destroy(probe->parser);
	BDBG_OBJECT_DESTROY(probe, bmp4_probe_t);
	BKNI_Free(probe);
	return;
}

static const bmedia_probe_stream *
b_mp4_probe_parse(bmedia_probe_base_t probe_, bfile_buffer_t buf, batom_pipe_t pipe, const bmedia_probe_parser_config *config)
{
	bmp4_probe_t probe = (bmp4_probe_t)probe_;
	off_t off;
	const size_t read_len = BMEDIA_PROBE_FEED_SIZE;
	bmp4_probe_stream *stream;

	BDBG_OBJECT_ASSERT(probe, bmp4_probe_t);
	BDBG_ASSERT(probe->stream==NULL);
	probe->track = NULL;
	probe->stream_error = false;
	probe->next_seek = 0;
	probe->movieheader_valid = false;
	probe->movie_valid = false;
    probe->mdat_valid = false;
    probe->sample_size_valid = false;
    bfile_segment_clear(&probe->compactsamplesize);
    bfile_segment_clear(&probe->samplesize);

	for(off=0;;) {
		batom_t atom;
		bfile_buffer_result result;
		size_t feed_len;
		size_t atom_len;
        bmp4_parser_status parser_status;


		if(probe->next_seek!=0) {
			off = probe->next_seek;
			bmp4_parser_seek(probe->parser, off); /* skip over large box */
			probe->next_seek = 0;
		}
		BDBG_MSG(("b_mp4_probe_parse: %p reading %u:%u", (void *)probe, (unsigned)(config->parse_offset + off), (unsigned)read_len));
		atom = bfile_buffer_read(buf, config->parse_offset+off, read_len, &result);
        if(!atom) {
            bmp4_parser_get_status(probe->parser, &parser_status);
            BDBG_MSG(("b_mp4_probe_parse: %p box_length:%u acc_length:%u", (void *)probe, (unsigned)parser_status.box_length, (unsigned)parser_status.acc_length));
            if(result!=bfile_buffer_result_eof || parser_status.box_length>parser_status.acc_length || parser_status.acc_length<BMP4_BOX_MAX_SIZE) {
                break;
            }
            atom_len = 0;
        }  else {
            atom_len = batom_len(atom);
            BDBG_MSG(("b_mp4_probe_parse: %p read %u:%u -> %p", (void *)probe, (unsigned)(config->parse_offset+off), (unsigned)atom_len, (void *)atom));
            off += atom_len;
            batom_pipe_push(pipe, atom);
        }
		feed_len = bmp4_parser_feed(probe->parser, pipe);
		if( (feed_len!=atom_len && probe->next_seek==0) || probe->stream_error) {
			break;
		}
		if(probe->movie_valid && (probe->mdat_valid || (probe->stream && probe->stream->fragmented))) {
			break;
		}
        if(!probe->sample_size_valid && probe->track) {
            bmp4_probe_track *track = probe->track;
            batom_cursor cursor;

            if(bfile_segment_test(&probe->samplesize)) {
	            bmp4_sample_size_header header;

                atom = bfile_buffer_read(buf, probe->samplesize.start, 128, &result);
                if(!atom) {
                    break;
                }
	            batom_cursor_from_atom(&cursor, atom);
                /* Only one of BMP4_SAMPLESIZE or BMP4_COMPACTSAMPLESIZE may be present */
                probe->sample_size_valid = bmp4_parse_sample_size_header(&cursor, &header);
                batom_release(atom);
                if (probe->sample_size_valid) {
                    track->sample_count = header.sample_count;
                }
                bfile_segment_clear(&probe->samplesize);
            } else if(bfile_segment_test(&probe->samplesize)) {
	            bmp4_compact_sample_size_header header;

                atom = bfile_buffer_read(buf, probe->compactsamplesize.start, 128, &result);
                if(!atom) {
                    break;
                }
	            batom_cursor_from_atom(&cursor, atom);
                probe->sample_size_valid = bmp4_parse_compact_sample_size_header(&cursor, &header);
                batom_release(atom);
                if (probe->sample_size_valid) {
                    track->sample_count = header.sample_count;
                }
            }
        }
        if(result==bfile_buffer_result_eof) {
            bmp4_parser_get_status(probe->parser, &parser_status);
            if(parser_status.box_length>parser_status.acc_length) {
                break;
            } /* else keep on sending dummy data */
        }
	}
    if(probe->track) {
        BKNI_Free(probe->track);
        probe->track=NULL;
    }
	bmp4_parser_reset(probe->parser);
	/* return result of parsing */
	stream = probe->stream;
    if(stream) {
        if(stream->media.duration) {
            stream->media.max_bitrate = ((1000*8)*((stream->mdat.size+stream->moov.size)))/stream->media.duration;
        }
		BDBG_MSG(("b_mp4_probe_parse: %#lx moov: %u:%u mdat:%u:%u %uKBps", (unsigned long)probe, (unsigned)stream->moov.offset, (unsigned)stream->moov.size, (unsigned)stream->mdat.offset, (unsigned)stream->mdat.size, stream->media.max_bitrate/1024));
    }
	probe->stream = NULL;
	return &stream->media;
}

static void
b_mp4_probe_stream_free(bmedia_probe_base_t probe_, bmedia_probe_stream *stream)
{
	bmp4_probe_t probe = (bmp4_probe_t)probe_;
	BDBG_OBJECT_ASSERT(probe, bmp4_probe_t);
	BSTD_UNUSED(probe);

	bmedia_probe_basic_stream_free(probe_, stream);
	return;
}


const bmedia_probe_format_desc bmp4_probe = {
	bstream_mpeg_type_mp4,
	b_mp4_ext, /* ext_list */
	256, /* ftyp box shall be smaller then that */
	b_mp4_probe_header_match, /* header_match */
	b_mp4_probe_create, /* create */
	b_mp4_probe_destroy, /* destroy */
	b_mp4_probe_parse, /* parse */
	b_mp4_probe_stream_free /* stream free */
};

