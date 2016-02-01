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
 * BMedia library, Probe of AAC audio with ADTS encapsulation
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "baac_adts_probe.h"
#include "bkni.h"
#include "biobits.h"
#include "baac_probe.h"
BDBG_MODULE(baac_adts_probe);


typedef struct bmedia_probe_audio_aac_adts *baac_adts_probe_t;
BDBG_OBJECT_ID(baac_adts_probe_t);

struct bmedia_probe_audio_aac_adts {
	BDBG_OBJECT(baac_adts_probe_t)
	batom_accum_t acc;
};

bmedia_probe_base_es_t 
b_aac_adts_probe_create(batom_factory_t factory)
{
	baac_adts_probe_t aac_adts;
	BDBG_ASSERT(factory);

	aac_adts = BKNI_Malloc(sizeof(*aac_adts));
	if(!aac_adts) {
		goto err_alloc;
	}
	BDBG_OBJECT_INIT(aac_adts, baac_adts_probe_t);
	aac_adts->acc = batom_accum_create(factory);
	if(!aac_adts->acc) {
		goto err_acc;
	}
	return (bmedia_probe_base_es_t)aac_adts;
err_acc:
	BKNI_Free(aac_adts);
err_alloc:
	return NULL;
}

void 
b_aac_adts_probe_destroy(bmedia_probe_base_es_t probe)
{
	baac_adts_probe_t aac_adts = (baac_adts_probe_t)probe;
	BDBG_OBJECT_ASSERT(aac_adts, baac_adts_probe_t);
	batom_accum_destroy(aac_adts->acc);
	BDBG_OBJECT_DESTROY(aac_adts, baac_adts_probe_t);
	BKNI_Free(aac_adts);
	return;
}

void 
b_aac_adts_probe_reset(bmedia_probe_base_es_t probe)
{
	baac_adts_probe_t aac_adts = (baac_adts_probe_t)probe;
	BDBG_OBJECT_ASSERT(aac_adts, baac_adts_probe_t);
	batom_accum_clear(aac_adts->acc);
	return;
}

/* ISO/IEC 13818-7:2005(E) */
/* Table 35 . Sampling frequency dependent on sampling_frequency_index */
static const unsigned b_aac_adts_sample_rate[]={
	96000,
	88200,
	64000,
	48000,
	44100,
	32000,
	24000,
	22050,
	16000,
	12000,
	11025,
	8000
};
/* Table 42 . Channel Configuration */

static const uint8_t b_aac_adts_channels[]={
	0,
	1,
	2,
	3,
	4,
	5,
	5+1,
	7+1
};


size_t
b_aac_adts_probe_parse_header(batom_cursor *cursor, bmedia_probe_audio *info, uint32_t sync_word)
{
	BDBG_MSG(("b_aac_adts_probe_parse_header: sync_word:%#x", sync_word));
	/* Table 8 . Syntax of adts_fixed_header() */
	if( 
		 	(B_GET_BIT(sync_word, 3)/* ID */ != 0  &&  B_GET_BITS(sync_word, 2, 1) /* layer */  == 0x00) /* MPEG-2 AAC, ISO/IEC 13818-7:2005(E) 8.1.3.2.2 Data Elements*/
		 || (B_GET_BIT(sync_word, 3)/* ID */ == 0  &&  B_GET_BITS(sync_word, 2, 1) /* layer */  != 0x3) /* MPEG-4 AAC,  ISO/IEC 14496-3:2005(E), 1.A.4.3 Audio Data Transport Stream (ADTS) */
		) {
		unsigned index;
		sync_word = batom_cursor_uint32_be(cursor);
		index = B_GET_BITS(sync_word, 29, 26); /* sampling_frequency_index */
		BDBG_MSG(("b_aac_adts_probe_parse_header: sampling_frequency_index:%u channel_configuration:%u aac_frame_length:%u", index,  B_GET_BITS(sync_word, 24, 22), B_GET_BITS(sync_word, 17, 5)));
		if(index<sizeof(b_aac_adts_sample_rate)/sizeof(*b_aac_adts_sample_rate)) {
            bmedia_probe_aac_audio codec_specific;
			info->sample_rate = b_aac_adts_sample_rate[index];
			index = B_GET_BITS(sync_word, 24, 22);
			info->channel_count = b_aac_adts_channels[index];
			info->sample_size = 16;

            /*  This configuration is based off of the ADTS profile_ObjectType ISO/IEC 14496-3 {ed 3.0}, pg  94. */
            index = (B_GET_BITS(sync_word, 31, 30) + 1);
            codec_specific.profile =  index < 5 ? (bmedia_probe_aac_profile) index : bmedia_probe_aac_profile_unknown;
            BKNI_Memcpy(&info->codec_specific, &codec_specific, sizeof(codec_specific));

			info->codec = baudio_format_aac;
			index = B_GET_BITS(sync_word, 17, 5); /* aac_frame_length */
			info->bitrate = (8*index * info->sample_rate) / (1024 /* AAC samples per frame */ * 1000 /* KBps */);
			if(index > (2+4)) {
				return index - (2 + 4);
			}
		}
	}
	return 0;
}

bmedia_probe_track *
b_aac_adts_probe_feed(bmedia_probe_base_es_t probe, batom_t atom, bool *done)
{
	baac_adts_probe_t aac_adts = (baac_adts_probe_t)probe;
	batom_cursor cursor;
	batom_accum_t acc;
    bmedia_probe_audio info;
	BSTD_UNUSED(done);


	BDBG_OBJECT_ASSERT(aac_adts, baac_adts_probe_t);
	acc = aac_adts->acc;
	batom_accum_add_atom(acc, atom);
    BKNI_Memset(&info, 0, sizeof(info)); /* this is not needed, used to fix warning-bug in GCC compiler */
	for(;;) {
		size_t byte_cnt=0;
		unsigned frame_cnt;
		bmedia_probe_track *track;
		uint32_t temp = 0;

		batom_cursor_from_accum(&cursor, acc);
		for(;;) {
			int byte;
			byte = batom_cursor_next(&cursor);
			if(byte==BATOM_EOF) {
				goto done;
			}
			byte_cnt++;
			temp = ((temp&0xFF)<<8)|byte;
			if(B_GET_BITS(temp,15, 4)==B_AAC_ADTS_SYNC) {
				break;
			}
		}
		/* found sync word, now look for back-to-back frames */
		for(frame_cnt=0;frame_cnt<B_AAC_ADTS_N_FRAMES;frame_cnt++) {
			BDBG_MSG(("b_aac_adts_probe_feed: %p sync_word %#x at:%u", (void *)aac_adts, temp, (unsigned)batom_cursor_pos(&cursor)-B_AAC_ADTS_SYNC_LEN));
			temp = b_aac_adts_probe_parse_header(&cursor,&info, temp);
			if(BATOM_IS_EOF(&cursor)) {
				goto done; /* wait for more data */
			} else if(temp>0) {
				BDBG_MSG(("b_aac_adts_probe_feed: %p header %u at:%u size:%u samplerate:%u channels:%u", (void *)aac_adts, frame_cnt, (unsigned)batom_cursor_pos(&cursor), temp, info.sample_rate, info.channel_count));
				batom_cursor_skip(&cursor, temp);
				temp = batom_cursor_uint16_be(&cursor); /* read next syncword */
				if(BATOM_IS_EOF(&cursor)) {
					goto done; /* wait for more data */
				}
				if(B_GET_BITS(temp, 15,4)==B_AAC_ADTS_SYNC) {
					continue;
				}
				BDBG_MSG(("b_aac_adts_probe_feed: %p out of sync at %u %#x-%#x(%#x)", (void *)aac_adts, (unsigned)batom_cursor_pos(&cursor)-B_AAC_ADTS_SYNC_LEN, temp, B_GET_BITS(temp,15,5), B_AAC_ADTS_SYNC));
			} 
			byte_cnt+=B_AAC_ADTS_SYNC_LEN;
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
		BDBG_MSG(("b_aac_adts_probe_feed: %p trim %u", (void *)aac_adts, (unsigned)byte_cnt));
		if(byte_cnt<=B_AAC_ADTS_SYNC_LEN) {
			break;
		}
		batom_cursor_from_accum(&cursor, acc);
		batom_cursor_skip(&cursor, byte_cnt-B_AAC_ADTS_SYNC_LEN);
		batom_accum_trim(acc, &cursor);
	}
	return NULL;
}

static const bmedia_probe_file_ext b_aac_adts_ext[] =  {
	{"aac"}, 
	{""}
};

const bmedia_probe_es_desc baac_adts_probe = {
	bmedia_track_type_audio,
	{
		baudio_format_mpeg
	},
	b_aac_adts_ext,
	(2*B_AAC_ADTS_N_FRAMES)*8192,
	b_aac_adts_probe_create,
	b_aac_adts_probe_destroy,
	b_aac_adts_probe_feed,
	b_aac_adts_probe_reset,
	bmedia_probe_es_nolast
};
	
