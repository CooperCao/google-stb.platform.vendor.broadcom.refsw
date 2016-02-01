/***************************************************************************
 *     Copyright (c) 2007 Broadcom Corporation
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
 * BMedia library, MPEG video elementary stream probe
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bmpeg_video_probe.h"
#include "bkni.h"
#include "biobits.h"
#include "bmedia_util.h"
BDBG_MODULE(bmpeg_video_probe);

#define BDBG_MSG_TRACE(x) /* BDBG_MSG(x)  */

#define MPEG_ERROR_COUNT_THRESHOLD 10

typedef struct bmedia_probe_video_mpeg_video *bmpeg_video_probe_t;
BDBG_OBJECT_ID(bmpeg_video_probe_t);

struct bmedia_probe_video_mpeg_video {
	BDBG_OBJECT(bmpeg_video_probe_t)
	batom_accum_t acc;
	uint16_t seq_scode;
    uint8_t last_scode;
	unsigned slice_scode;
	unsigned pic_scode;
	unsigned ext_scode;
	unsigned gop_scode;
	unsigned err_cnt;
	bmedia_probe_video info;
    bmedia_probe_mpeg_video mpeg_video_info; /* keep them separate to prevent pointer aliasing */
};

static void 
b_mpeg_video_probe_reset(bmedia_probe_base_es_t probe)
{
	bmpeg_video_probe_t mpeg_video = (bmpeg_video_probe_t)probe;
	BDBG_OBJECT_ASSERT(mpeg_video, bmpeg_video_probe_t);
	batom_accum_clear(mpeg_video->acc);
    bmedia_probe_video_init(&mpeg_video->info);
    mpeg_video->mpeg_video_info = *(bmedia_probe_mpeg_video*)&mpeg_video->info.codec_specific;
	mpeg_video->seq_scode = 0;
	mpeg_video->pic_scode = 0;
	mpeg_video->ext_scode = 0;
	mpeg_video->gop_scode = 0;
	mpeg_video->slice_scode = 0;
	mpeg_video->err_cnt=0;
    mpeg_video->last_scode = 0xFF; /* invalid */
	return;
}

static bmedia_probe_base_es_t 
b_mpeg_video_probe_create(batom_factory_t factory)
{
	bmpeg_video_probe_t mpeg_video;
	BDBG_ASSERT(factory);

	mpeg_video = BKNI_Malloc(sizeof(*mpeg_video));
	if(!mpeg_video) {
		goto err_alloc;
	}
	BDBG_OBJECT_INIT(mpeg_video, bmpeg_video_probe_t);
	mpeg_video->acc = batom_accum_create(factory);
	if(!mpeg_video->acc) {
		goto err_acc;
	}
	b_mpeg_video_probe_reset((bmedia_probe_base_es_t)mpeg_video);
	return (bmedia_probe_base_es_t)mpeg_video;
err_acc:
	BKNI_Free(mpeg_video);
err_alloc:
	return NULL;
}

static void 
b_mpeg_video_probe_destroy(bmedia_probe_base_es_t probe)
{
	bmpeg_video_probe_t mpeg_video = (bmpeg_video_probe_t)probe;
	BDBG_OBJECT_ASSERT(mpeg_video, bmpeg_video_probe_t);
	batom_accum_destroy(mpeg_video->acc);
	BDBG_OBJECT_DESTROY(mpeg_video, bmpeg_video_probe_t);
	BKNI_Free(mpeg_video);
	return;
}

static bool
b_mpeg_video_parse_sequence(batom_cursor *cursor, bmedia_probe_video *info, bmedia_probe_mpeg_video *mpeg_video_info)
{
	uint32_t temp, bitrate;
    unsigned frame_rate_code;
    unsigned bitrate_value;

    static const unsigned  frame_rates[16] = {
        0,      /* 0 */
        23976,  /* 1 */
        24000,  /* 2 */
        25000,  /* 3 */
        29970,  /* 4 */
        30000,  /* 5 */
        50000,  /* 6 */
        59940,  /* 7 */
        60000,  /* 8 */
        0,      /* 9 */
        0,      /* 10 */
        0,      /* 11 */
        0,      /* 12 */
        0,      /* 13 */
        0,      /* 14 */
        0       /* 15 */
    };

	/* ISO/IEC 13818-2 : 2000 (E), Table 6.2.2.1 Sequence header */
	temp = batom_cursor_uint32_be(cursor);
	bitrate = batom_cursor_uint32_be(cursor);
    if(BATOM_IS_EOF(cursor)) {return false;}
    bmedia_probe_video_init(info);
	info->width = B_GET_BITS(temp, 31, 20);
	info->height = B_GET_BITS(temp, 19, 8);	
    bitrate_value = B_GET_BITS(bitrate, 31, 14);
    if(bitrate_value!=B_GET_BITS(0xFFFFFFFFul, 31, 14)) {
	    info->bitrate = bitrate_value;
    } else {
        info->bitrate = 0;
    }
    frame_rate_code = B_GET_BITS(temp, 3, 0);
	if(B_GET_BITS(temp, 7, 4)==0 || /* Table 6-3 . aspect_ratio_information */
	   frame_rate_code==0    /* Table 6-4 . frame_rate_value */
			) { 
		return false;
	}
    mpeg_video_info->framerate = frame_rates[frame_rate_code];
	return true;
}

static bmedia_probe_track *
b_mpeg_video_probe_make_track(bmpeg_video_probe_t mpeg_video)
{
	bmedia_probe_track *track;
	track = BKNI_Malloc(sizeof(*track));
	batom_accum_clear(mpeg_video->acc);
	/* found all frames */
	if(track) {
        mpeg_video->mpeg_video_info.seq_scode_count = mpeg_video->seq_scode;
        mpeg_video->mpeg_video_info.pic_scode_count = mpeg_video->pic_scode;

		bmedia_probe_track_init(track);
		if(mpeg_video->ext_scode) {
			mpeg_video->info.codec = bvideo_codec_mpeg2;			
		} else {
			mpeg_video->info.codec = bvideo_codec_mpeg1;			
		}
		mpeg_video->info.bitrate = (mpeg_video->info.bitrate * 2)/5; /* (bitrate * 400)/1000 */
		mpeg_video->info.timestamp_order = bmedia_timestamp_order_display;

		track->type = bmedia_track_type_video;
        *(bmedia_probe_mpeg_video *)&mpeg_video->info.codec_specific = mpeg_video->mpeg_video_info;
		track->info.video = mpeg_video->info;

	}
	return track;
}

static bmedia_probe_track *
b_mpeg_video_probe_feed(bmedia_probe_base_es_t probe, batom_t atom, bool *done)
{
	bmpeg_video_probe_t mpeg_video = (bmpeg_video_probe_t)probe;
	batom_accum_t acc;
	batom_cursor cursor;
	size_t pos = 0;
	bmedia_probe_video info;
    bmedia_probe_mpeg_video mpeg_video_info;

	BDBG_OBJECT_ASSERT(mpeg_video, bmpeg_video_probe_t);

	acc = mpeg_video->acc;
	batom_accum_add_atom(acc, atom);
	batom_cursor_from_accum(&cursor, acc);
	for(;;) {
		uint32_t scode;
		bool valid;
        unsigned last_scode = mpeg_video->last_scode;


		scode = bmedia_video_scan_scode(&cursor, 0xFFFFFFFFul);
		pos = batom_cursor_pos(&cursor);
		BDBG_MSG_TRACE(("b_mpeg_video_probe_feed: %#lx scode %#x at %u", (unsigned long)mpeg_video, scode, pos));
		if(scode==0) {
			goto eof;
		}
        scode &= 0xFF;
        mpeg_video->last_scode = scode;
		switch(scode) {
		case 0x00:
			mpeg_video->pic_scode++;
			break;
		case 0xB5:
            mpeg_video->ext_scode++;
            if(last_scode == 0xB3) {
                /* Parse the sequence extension */
                uint32_t tempHi = batom_cursor_uint32_be(&cursor);
                uint16_t tempLo = batom_cursor_uint16_be(&cursor);
                if(!BATOM_IS_EOF(&cursor)) {
                    uint8_t profile_and_level = B_GET_BITS(tempHi, 28, 20);
                    mpeg_video->mpeg_video_info.profile = B_GET_BITS(profile_and_level, 6, 4);
                    mpeg_video->mpeg_video_info.level = B_GET_BITS(profile_and_level, 3, 0);
                    mpeg_video->mpeg_video_info.progressive_sequence = B_GET_BIT(tempHi, 19);
                    mpeg_video->mpeg_video_info.low_delay = B_GET_BIT(tempLo, 7);
                    mpeg_video->mpeg_video_info.valid = true;
                    mpeg_video->info.bitrate |= B_GET_BITS(tempHi, 12, 1)<<18;
                }
            }
            break;
		case 0xB8:
			mpeg_video->gop_scode++;
			break;
		case 0xB3:
            mpeg_video_info = mpeg_video->mpeg_video_info;
			valid = b_mpeg_video_parse_sequence(&cursor, &info, &mpeg_video_info);
			if(BATOM_IS_EOF(&cursor)) {
				goto eof;
			}
			BDBG_MSG(("b_mpeg_video_probe_feed: %#lx video %ux%u", (unsigned long)mpeg_video, info.width, info.height));
			if(!valid) {
				*done = true;
				batom_accum_clear(acc);
				return NULL;
			}
			/* have located two sequence headers */
			mpeg_video->seq_scode++;
			BDBG_MSG(("b_mpeg_video_probe_feed: %#lx sequence:%u gop:%u picture:%u extension:%u", (unsigned long)mpeg_video, mpeg_video->seq_scode, mpeg_video->gop_scode, mpeg_video->pic_scode, mpeg_video->ext_scode));
			if(mpeg_video->seq_scode==1) {
				mpeg_video->pic_scode = 0;
				mpeg_video->ext_scode = 0;
				mpeg_video->gop_scode = 0;
				mpeg_video->info = info;
				mpeg_video->mpeg_video_info = mpeg_video_info;
			} else {
				if(info.width != mpeg_video->info.width || info.height != mpeg_video->info.height) {
					mpeg_video->seq_scode=0;
					break;
				}
				if(mpeg_video->gop_scode<mpeg_video->seq_scode && mpeg_video->pic_scode>=mpeg_video->seq_scode) {
					return b_mpeg_video_probe_make_track(mpeg_video);
				}
			}
			break;
		case 0xB7:
			/* EOS */
			if(batom_cursor_next(&cursor)==BATOM_EOF && mpeg_video->seq_scode>0 && mpeg_video->pic_scode>0) {
                return b_mpeg_video_probe_make_track(mpeg_video);
			}
			break;
		case 0xB0:
		case 0xB1:
			*done = true;
			BDBG_MSG(("b_mpeg_video_probe_feed: %#lx found scode %#x not a video ES", (unsigned long)mpeg_video, scode));
			goto flush;
		default:
			if(scode>=0x01  && scode<=0xAF) {
				mpeg_video->slice_scode++;
			} else if(scode>=0xB9) { /* system start codes */
				mpeg_video->err_cnt++;
				if(mpeg_video->err_cnt > MPEG_ERROR_COUNT_THRESHOLD){
					*done = true;
					BDBG_MSG(("b_mpeg_video_probe_feed: %#lx found scode %#x not a video ES", (unsigned long)mpeg_video, scode));
					goto flush;
				}
			}
			break;
		}
		continue;
	}
eof:
	BDBG_MSG(("b_mpeg_video_probe_feed: %p trim %u", (void *)mpeg_video, (unsigned)pos));
	if(pos>B_MEDIA_VIDEO_SCODE_LEN) {
		batom_cursor_from_accum(&cursor, acc);
		batom_cursor_skip(&cursor, pos-B_MEDIA_VIDEO_SCODE_LEN);
		batom_accum_trim(acc, &cursor);
	}
done:
	return NULL;
flush:
	batom_accum_clear(acc);
	goto done;
}

static bmedia_probe_track *
b_mpeg_video_probe_last(bmedia_probe_base_es_t probe, unsigned *probability)
{
	bmpeg_video_probe_t mpeg_video = (bmpeg_video_probe_t)probe;

	BDBG_OBJECT_ASSERT(mpeg_video, bmpeg_video_probe_t);

    BDBG_MSG(("b_mpeg_video_probe_last: %#lx %u %u %u %u", (unsigned long)mpeg_video,  mpeg_video->gop_scode, mpeg_video->seq_scode, mpeg_video->pic_scode, mpeg_video->slice_scode));
	if(mpeg_video->gop_scode>0 && mpeg_video->seq_scode>0 && mpeg_video->pic_scode>0 && (2*(mpeg_video->slice_scode+1))>=mpeg_video->pic_scode) {
		*probability = 60;
		return b_mpeg_video_probe_make_track(mpeg_video);
	} else {
		return NULL;
	}
}

static const bmedia_probe_file_ext b_mpeg_video_ext[] =  {
	{"es"}, {"ves"}, {"mpg"},
	{""}
};

const bmedia_probe_es_desc bmpeg_video_probe = {
	bmedia_track_type_video,
	{
		bvideo_codec_mpeg2
	},
	b_mpeg_video_ext,
	((20*(1000*1000))/8)*2, /* parse 2 seconds of 20MBps stream */
	b_mpeg_video_probe_create,
	b_mpeg_video_probe_destroy,
	b_mpeg_video_probe_feed,
	b_mpeg_video_probe_reset,
	b_mpeg_video_probe_last
};
	
