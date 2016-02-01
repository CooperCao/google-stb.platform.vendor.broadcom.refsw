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
 * BMedia library, AVS video elementary stream probe
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bavs_video_probe.h"
#include "bkni.h"
#include "biobits.h"
#include "bmedia_util.h"

BDBG_MODULE(bavs_video_probe);

#define BDBG_MSG_TRACE(x) /* BDBG_MSG(x)  */

#define AVS_ERROR_COUNT_THRESHOLD 10

typedef struct bmedia_probe_video_avs_video *bavs_video_probe_t;

BDBG_OBJECT_ID(bavs_video_probe_t);

struct bmedia_probe_video_avs_video {
	BDBG_OBJECT(bavs_video_probe_t)
	batom_accum_t acc;
	uint16_t seq_scode;
	unsigned slice_scode;
	unsigned pic_scode;
	unsigned ext_scode;
	unsigned err_cnt;
	bmedia_probe_video info;
};

static void 
b_avs_video_probe_reset(bmedia_probe_base_es_t probe)
{
	bavs_video_probe_t avs_video = (bavs_video_probe_t)probe;
	BDBG_OBJECT_ASSERT(avs_video, bavs_video_probe_t);
	batom_accum_clear(avs_video->acc);
	bmedia_probe_video_init(&avs_video->info);
	avs_video->seq_scode = 0;
	avs_video->pic_scode = 0;
	avs_video->ext_scode = 0;
	avs_video->slice_scode = 0;
	avs_video->err_cnt=0;
	return;
}

static bmedia_probe_base_es_t 
b_avs_video_probe_create(batom_factory_t factory)
{
	bavs_video_probe_t avs_video;
	BDBG_ASSERT(factory);

	avs_video = BKNI_Malloc(sizeof(*avs_video));
	if(!avs_video) {
		goto err_alloc;
	}
	BDBG_OBJECT_INIT(avs_video, bavs_video_probe_t);
	avs_video->acc = batom_accum_create(factory);
	if(!avs_video->acc) {
		goto err_acc;
	}
	b_avs_video_probe_reset((bmedia_probe_base_es_t)avs_video);
	return (bmedia_probe_base_es_t)avs_video;
err_acc:
	BKNI_Free(avs_video);
err_alloc:
	return NULL;
}

static void 
b_avs_video_probe_destroy(bmedia_probe_base_es_t probe)
{
	bavs_video_probe_t avs_video = (bavs_video_probe_t)probe;
	BDBG_OBJECT_ASSERT(avs_video, bavs_video_probe_t);
	batom_accum_destroy(avs_video->acc);
	BDBG_OBJECT_DESTROY(avs_video, bavs_video_probe_t);
	BKNI_Free(avs_video);
	return;
}

static bool
b_avs_video_parse_sequence(batom_cursor *cursor, bmedia_probe_video *info)
{
    uint32_t temp1, temp2, bitrate_lower, bitrate_higher;
    unsigned frame_rate_code, aspect_ratio_code;
    unsigned bitrate_value;
    bmedia_probe_avs_video avs_video;

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

    bmedia_probe_video_init(info);
    temp1 = batom_cursor_uint32_be(cursor);
    info->width = B_GET_BITS(temp1, 14, 1);
    temp2 = batom_cursor_uint32_be(cursor);
    info->height = B_GET_BITS(temp2, 31, 19) | B_GET_BIT(temp1, 0) <<13;
    aspect_ratio_code = B_GET_BITS(temp2, 13, 10);
    frame_rate_code = B_GET_BITS(temp2, 9, 6);

    temp1 = batom_cursor_uint32_be(cursor);   
    bitrate_lower = B_GET_BITS(temp1, 31, 20) | B_GET_BITS(temp2, 5, 0) << 11;
    bitrate_higher = B_GET_BITS(temp1, 18, 7);
    
    bitrate_value = (bitrate_higher<<18) | bitrate_lower;
    
    info->bitrate = (400*bitrate_value)/1024;
       
    if(aspect_ratio_code==0 || frame_rate_code==0) { 
	return false;
    }

    avs_video.framerate = frame_rates[frame_rate_code];
    BKNI_Memcpy(&info->codec_specific, &avs_video, sizeof(avs_video));
    return true;
}

static bmedia_probe_track *
b_avs_video_probe_make_track(bavs_video_probe_t avs_video)
{
	bmedia_probe_track *track;
	uint64_t bitrate;
	track = BKNI_Malloc(sizeof(*track));
	batom_accum_clear(avs_video->acc);
	/* found all frames */
	if(track) {
		bmedia_probe_track_init(track);		
		avs_video->info.codec = bvideo_codec_avs;					
		bitrate = (uint64_t)avs_video->info.bitrate*400;
		avs_video->info.bitrate = bitrate/1000;
		avs_video->info.timestamp_order = bmedia_timestamp_order_display;
		track->type = bmedia_track_type_video;
		track->info.video = avs_video->info;
	}
	return track;
}

static bmedia_probe_track *
b_avs_video_probe_feed(bmedia_probe_base_es_t probe, batom_t atom, bool *done)
{
	bavs_video_probe_t avs_video = (bavs_video_probe_t)probe;
	batom_accum_t acc;
	batom_cursor cursor;
	size_t pos = 0;
	bmedia_probe_video info;

	BDBG_OBJECT_ASSERT(avs_video, bavs_video_probe_t);

	acc = avs_video->acc;
	batom_accum_add_atom(acc, atom);
	batom_cursor_from_accum(&cursor, acc);
	for(;;) {
		uint32_t scode;
		bool valid;


		scode = bmedia_video_scan_scode(&cursor, 0xFFFFFFFFul);
		pos = batom_cursor_pos(&cursor);
		BDBG_MSG_TRACE(("b_avs_video_probe_feed: %#lx scode %#x at %u", (unsigned long)avs_video, scode, pos));
		if(scode==0) {
			goto eof;
		}
		switch(scode) {
		case 0x1B3:
		case 0x1B6:
			avs_video->pic_scode++;
			break;
		case 0x1B5:
			avs_video->ext_scode++;					
			break;		
		case 0x1B0:
			valid = b_avs_video_parse_sequence(&cursor, &info);
			if(BATOM_IS_EOF(&cursor)) {
				goto eof;
			}
			BDBG_MSG(("b_avs_video_probe_feed: %#lx video %ux%u", (unsigned long)avs_video, info.width, info.height));
			if(!valid) {
				*done = true;
				batom_accum_clear(acc);
				return NULL;
			}
			/* have located two sequence headers */
			avs_video->seq_scode++;
			BDBG_MSG(("b_avs_video_probe_feed: %#lx sequence:%u picture:%u extension:%u", (unsigned long)avs_video, avs_video->seq_scode, avs_video->pic_scode, avs_video->ext_scode));
			if(avs_video->seq_scode==1) {
				avs_video->pic_scode = 0;
				avs_video->ext_scode = 0;				
				avs_video->info = info;
			} else {
				if(info.width != avs_video->info.width || info.height != avs_video->info.height) {
					avs_video->seq_scode=0;
					break;
				}
				if(avs_video->pic_scode>=avs_video->seq_scode) {
					return b_avs_video_probe_make_track(avs_video);
				}
			}
			break;
		case 0x1B1:
			/* EOS */
			if(batom_cursor_next(&cursor)==BATOM_EOF && avs_video->seq_scode>0 && avs_video->pic_scode>0) {
			    return b_avs_video_probe_make_track(avs_video);
			}
			break;
		case 0x1B4:
		case 0x1B8:
			*done = true;
			BDBG_MSG(("b_avs_video_probe_feed: %#lx found scode %#x not a video ES", (unsigned long)avs_video, scode));
			goto flush;
		default:
			if(scode>=0x101  && scode<=0x1AF) {
				avs_video->slice_scode++;
			} else if(scode>=0x1B9) { /* system start codes */
				avs_video->err_cnt++;
				if(avs_video->err_cnt > AVS_ERROR_COUNT_THRESHOLD){
					*done = true;
					BDBG_MSG(("b_avs_video_probe_feed: %#lx found scode %#x not a video ES", (unsigned long)avs_video, scode));
					goto flush;
				}
			}
			break;
		}
		continue;
	}
eof:
	BDBG_MSG(("b_avs_video_probe_feed: %p trim %u", (void *)avs_video, (unsigned)pos));
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
b_avs_video_probe_last(bmedia_probe_base_es_t probe, unsigned *probability)
{
    bavs_video_probe_t avs_video = (bavs_video_probe_t)probe;
    
    BDBG_OBJECT_ASSERT(avs_video, bavs_video_probe_t);
    
    BDBG_MSG(("b_avs_video_probe_last: %#lx %u %u %u", (unsigned long)avs_video, avs_video->seq_scode, avs_video->pic_scode, avs_video->slice_scode));
    if(avs_video->seq_scode>0 && avs_video->pic_scode>0 && (2*(avs_video->slice_scode+1))>=avs_video->pic_scode) {
	*probability = 60;
	return b_avs_video_probe_make_track(avs_video);
    } else {
	return NULL;
    }
}

static const bmedia_probe_file_ext b_avs_video_ext[] =  {
	{"es"}, {"ves"}, {"mpg"}, {"avs"},
	{""}
};

const bmedia_probe_es_desc bavs_video_probe = {
	bmedia_track_type_video,
	{
		bvideo_codec_avs
	},
	b_avs_video_ext,
	((20*(1000*1000))/8)*2, /* parse 2 seconds of 20MBps stream */
	b_avs_video_probe_create,
	b_avs_video_probe_destroy,
	b_avs_video_probe_feed,
	b_avs_video_probe_reset,
	b_avs_video_probe_last
};
	
