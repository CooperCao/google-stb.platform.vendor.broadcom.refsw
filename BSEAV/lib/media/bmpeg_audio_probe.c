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
 * BMedia library, MPEG audio elementary stream probe
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bmpeg_audio_probe.h"
#include "bkni.h"
#include "biobits.h"
BDBG_MODULE(bmpeg_audio_probe);

#define BDBG_MSG_TRACE(x)   /* BDBG_MSG(x) */
#define B_MPEG_AUDIO_N_FRAMES	3

typedef struct bmedia_probe_audio_mpeg_audio *bmpeg_audio_probe_t;
BDBG_OBJECT_ID(bmpeg_audio_probe_t);

struct bmedia_probe_audio_mpeg_audio {
	BDBG_OBJECT(bmpeg_audio_probe_t)
	batom_accum_t acc;
};

bmedia_probe_base_es_t 
b_mpeg_audio_probe_create(batom_factory_t factory)
{
	bmpeg_audio_probe_t mpeg_audio;
	BDBG_ASSERT(factory);
	mpeg_audio = BKNI_Malloc(sizeof(*mpeg_audio));
    BDBG_MSG_TRACE(("b_mpeg_audio_probe_create: %#lx", (unsigned long)mpeg_audio));
	if(!mpeg_audio) {
		goto err_alloc;
	}
	BDBG_OBJECT_INIT(mpeg_audio, bmpeg_audio_probe_t);
	mpeg_audio->acc = batom_accum_create(factory);
	if(!mpeg_audio->acc) {
		goto err_acc;
	}
	return (bmedia_probe_base_es_t)mpeg_audio;
err_acc:
	BKNI_Free(mpeg_audio);
err_alloc:
	return NULL;
}

void 
b_mpeg_audio_probe_destroy(bmedia_probe_base_es_t probe)
{
	bmpeg_audio_probe_t mpeg_audio = (bmpeg_audio_probe_t)probe;
    BDBG_MSG_TRACE(("b_mpeg_audio_probe_destroy: %#lx", (unsigned long)probe));
	BDBG_OBJECT_ASSERT(mpeg_audio, bmpeg_audio_probe_t);
	batom_accum_destroy(mpeg_audio->acc);
	BDBG_OBJECT_DESTROY(mpeg_audio, bmpeg_audio_probe_t);
	BKNI_Free(mpeg_audio);
	return;
}

void 
b_mpeg_audio_probe_reset(bmedia_probe_base_es_t probe)
{
	bmpeg_audio_probe_t mpeg_audio = (bmpeg_audio_probe_t)probe;
    BDBG_MSG_TRACE(("b_mpeg_audio_probe_reset: %#lx", (unsigned long)probe));
	BDBG_OBJECT_ASSERT(mpeg_audio, bmpeg_audio_probe_t);
	batom_accum_clear(mpeg_audio->acc);
	return;
}

bmedia_probe_track *
b_mpeg_audio_probe_feed(bmedia_probe_base_es_t probe, batom_t atom, bool *done)
{
    bmedia_mpeg_audio_info mpeg_info;
    BSTD_UNUSED(done);
    return bmpeg_audio_probe_feed(probe, atom, &mpeg_info, NULL);
}

bmedia_probe_track *
bmpeg_audio_probe_feed(bmedia_probe_base_es_t probe, batom_t atom, bmedia_mpeg_audio_info *mpeg_info, bmp3_vbr_frame_info *vbr_info)
{
	bmpeg_audio_probe_t mpeg_audio = (bmpeg_audio_probe_t)probe;
	batom_cursor cursor;
	batom_accum_t acc;
    bmedia_probe_audio info;

    BDBG_MSG_TRACE(("b_mpeg_audio_probe_feed: %#lx", (unsigned long)probe));
	BDBG_OBJECT_ASSERT(mpeg_audio, bmpeg_audio_probe_t);
    BDBG_ASSERT(mpeg_info);
	acc = mpeg_audio->acc;
	batom_accum_add_atom(acc, atom);
    BKNI_Memset(&info, 0, sizeof(info)); /* this is not needed, used to fix warning-bug in GCC compiler */
	for(;;) {
		size_t byte_cnt=0;
		unsigned frame_cnt;
		bmedia_probe_track *track;
		uint16_t sync = 0;
        unsigned temp;

		batom_cursor_from_accum(&cursor, acc);
		for(;;) {
			int byte;
			byte = batom_cursor_next(&cursor);
			if(byte==BATOM_EOF) {
				goto done;
			}
			byte_cnt++;
			sync = ((sync&0xFF)<<8)|byte;
			if(B_GET_BITS(sync, 15,5)==B_MPEG_AUDIO_SYNC) {
				break;
			}
		}
		/* found sync word, now look for back-to-back frames */
		for(frame_cnt=0;frame_cnt<B_MPEG_AUDIO_N_FRAMES;frame_cnt++) {
			BDBG_MSG(("b_mpeg_audio_probe_feed: %p sync_word %#x at:%u", (void *)mpeg_audio, sync, (unsigned)batom_cursor_pos(&cursor)-B_MPEG_AUDIO_SYNC_LEN));
			temp = bmpeg_audio_probe_parse_header(&cursor, sync, &info, mpeg_info);
            /* Parse any necessary information from the frame data */
            if (frame_cnt==0 && vbr_info) {
                batom_cursor info_cursor;

                BATOM_CLONE(&info_cursor,&cursor);
                bmp3_parse_vbr_frame_info(&info_cursor, mpeg_info, vbr_info);
            }
			if(BATOM_IS_EOF(&cursor)) {
				goto done; /* wait for more data */
			} else if(temp>0) {
				BDBG_MSG(("b_mpeg_audio_probe_feed: %p header %u at:%u size:%u samplerate:%u bitrate:%u", (void *)mpeg_audio, frame_cnt, (unsigned)batom_cursor_pos(&cursor), temp, info.sample_rate, info.bitrate));
				batom_cursor_skip(&cursor, temp);
				temp = batom_cursor_uint16_be(&cursor);
				if(BATOM_IS_EOF(&cursor)) {
					goto done; /* wait for more data */
				}
				if(B_GET_BITS(temp, 15,5)==B_MPEG_AUDIO_SYNC) {
					continue;
				}
				BDBG_MSG(("b_mpeg_audio_probe_feed: %p out of sync at %u %#x-%#x(%#x)", (void *)mpeg_audio, (unsigned)batom_cursor_pos(&cursor)-B_MPEG_AUDIO_SYNC_LEN, temp, B_GET_BITS(temp,15,5), B_MPEG_AUDIO_SYNC));
			} 
			byte_cnt+=B_MPEG_AUDIO_SYNC_LEN;
			goto done; /* skip over sync word */
        }
		/* have found enough back-to-back frames */
		batom_accum_clear(acc);
		/* found all frames */
		track = BKNI_Malloc(sizeof(*track));
		if(track) {
			bmedia_probe_track_init(track);
			track->type = bmedia_track_type_audio;
			track->info.audio = info;
		}
		return track;
done:
		BDBG_MSG(("b_mpeg_audio_probe_feed: %p trim %u", (void *)mpeg_audio, (unsigned)byte_cnt));
		if(byte_cnt<=B_MPEG_AUDIO_SYNC_LEN) {
			break;
		}
		batom_cursor_from_accum(&cursor, acc);
		batom_cursor_skip(&cursor, byte_cnt-B_MPEG_AUDIO_SYNC_LEN);
		batom_accum_trim(acc, &cursor);
	}
	return NULL;
}

static const bmedia_probe_file_ext b_mpeg_audio_ext[] =  {
	{"mp2"}, {"mp3"}, {"mpg"},
	{""}
};

const bmedia_probe_es_desc bmpeg_audio_probe = {
	bmedia_track_type_audio,
	{
		baudio_format_mpeg
	},
	b_mpeg_audio_ext,
	(2*B_MPEG_AUDIO_N_FRAMES)*1344,
	b_mpeg_audio_probe_create,
	b_mpeg_audio_probe_destroy,
	b_mpeg_audio_probe_feed,
	b_mpeg_audio_probe_reset,
	bmedia_probe_es_nolast
};
	
