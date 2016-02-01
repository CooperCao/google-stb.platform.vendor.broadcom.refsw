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
 * BMedia library, DTS elementary stream probe
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/


#include "bstd.h"
#include "bdts_probe.h"
#include "bkni.h"
#include "biobits.h"

BDBG_MODULE(bdts_probe);

#define B_DTS_N_FRAMES	3
#define B_DTS_SYNC	0x7FFE8001
#define B_DTS_SYNC_LEN	4
#define B_DTS_SUBSTREAM_SYNC 0x64582025


typedef struct bmedia_probe_audio_dts *bdts_probe_t;
BDBG_OBJECT_ID(bdts_probe_t);

struct bmedia_probe_audio_dts {
	BDBG_OBJECT(bdts_probe_t)
	batom_accum_t acc;
};

static bmedia_probe_base_es_t
b_dts_probe_create(batom_factory_t factory)
{
	bdts_probe_t dts;
	BDBG_ASSERT(factory);

	dts = BKNI_Malloc(sizeof(*dts));
	if(!dts) {
		goto err_alloc;
	}
	BDBG_OBJECT_INIT(dts, bdts_probe_t);
	dts->acc = batom_accum_create(factory);
	if(!dts->acc) {
		goto err_acc;
	}
	return (bmedia_probe_base_es_t)dts;
err_acc:
	BKNI_Free(dts);
err_alloc:
	return NULL;
}

static void
b_dts_probe_destroy(bmedia_probe_base_es_t probe)
{
	bdts_probe_t dts = (bdts_probe_t)probe;
	BDBG_OBJECT_ASSERT(dts, bdts_probe_t);
	batom_accum_destroy(dts->acc);
	BDBG_OBJECT_DESTROY(dts, bdts_probe_t);
	BKNI_Free(dts);
	return;
}

static void
b_dts_probe_reset(bmedia_probe_base_es_t probe)
{
	bdts_probe_t dts = (bdts_probe_t)probe;
	BDBG_OBJECT_ASSERT(dts, bdts_probe_t);
	batom_accum_clear(dts->acc);
	return;
}

static const uint16_t b_dts_sample_rate[]= {
	/* 0000 */ 0,
	/* 0001 */ 8000,
	/* 0010 */ 16000,
	/* 0011 */ 32000,
	/* 0100 */ 0,
	/* 0101 */ 0,
	/* 0110 */ 11025,
	/* 0111 */ 22050,
	/* 1000 */ 44100,
	/* 1001 */ 0,
	/* 1010 */ 0,
	/* 1011 */ 12000,
	/* 1100 */ 24000,
	/* 1101 */ 48000,
	/* 1110 */ 0,
	/* 1111 */ 0
};

static const uint16_t b_dts_bitrate[]= {
	/* 00000 */ 32,
	/* 00001 */ 56,
	/* 00010 */ 64,
	/* 00011 */ 96,
	/* 00100 */ 112,
	/* 00101 */ 128,
	/* 00110 */ 192,
	/* 00111 */ 224,
	/* 01000 */ 256,
	/* 01001 */ 320,
	/* 01010 */ 384,
	/* 01011 */ 448,
	/* 01100 */ 512,
	/* 01101 */ 576,
	/* 01110 */ 640,
	/* 01111 */ 768,
	/* 10000 */ 960,
	/* 10001 */ 1024,
	/* 10010 */ 1152,
	/* 10011 */ 1280,
	/* 10100 */ 1344,
	/* 10101 */ 1408,
	/* 10110 */ 1411,
	/* 10111 */ 1472,
	/* 11000 */ 1536,
	/* 11001 */ 1920,
	/* 11010 */ 2048,
	/* 11011 */ 3072,
	/* 11100 */ 3840,
	/* 11101 */ 0, /* open */
	/* 11110 */ 0, /* variable */
	/* 11111 */ 0, /* loss-less */
};

static const uint8_t b_dts_nfchans[] = {
	/* 000000 */ 1,
	/* 000001 */ 2,
	/* 000010 */ 2,
	/* 000011 */ 2,
	/* 000100 */ 2,
	/* 000101 */ 3,
	/* 000110 */ 3,
	/* 000111 */ 4,
	/* 001000 */ 4,
	/* 001001 */ 5,
	/* 001010 */ 6,
	/* 001011 */ 6,
	/* 001100 */ 6,
	/* 001101 */ 7,
	/* 001110 */ 8,
	/* 001111 */ 8
};

/* Source PCM Resolution (Table 5.19 from ETSI TS 102 114 V.1.2.1) */
static const uint8_t b_dts_sample_size[] = {
	/* 000 */ 16,
	/* 001 */ 16,
	/* 010 */ 20,
	/* 011 */ 20,
	/* 100 */ 0, /* invalid */
	/* 101 */ 24,
	/* 110 */ 24,
	/* 111 */ 0 /* invalid */
};


static size_t
b_dts_probe_parse_header(batom_cursor *cursor, bmedia_probe_audio *info)
{
	uint32_t temp,data = batom_cursor_uint32_be(cursor);
	unsigned frmsize = B_GET_BITS(data,17,4) + 1;
	bool crc_present = false;
	
	/* Valid range is 96 - 16384 */
	if(frmsize<96 || frmsize>16384){
		return 0;
	}

	frmsize -= (B_DTS_SYNC_LEN+4+4+4);
	
	if(B_GET_BITS(data,31,26) == 0x3F){
		info->codec = baudio_format_dts;	
		crc_present = B_GET_BIT(data, 25);

		/* skip 22 bits after sync word */
		temp = (B_GET_BITS(data, 3, 0))<<2;
		data = batom_cursor_uint32_be(cursor);
		temp |= B_GET_BITS(data, 31, 30);
		if(temp > 0xF){
			return 0;
		}
		info->channel_count = b_dts_nfchans[temp];
		info->sample_rate = b_dts_sample_rate[B_GET_BITS(data, 29, 26)];
		if(!info->sample_rate){
			return 0;
		}
		temp = B_GET_BITS(data, 25, 21);
		if(temp > 0x1F){
			return 0;
		}
		info->bitrate = b_dts_bitrate[temp];

        /* LFE present */
        temp = B_GET_BITS(data, 10, 9);
        if (temp == 1 || temp == 2) {
            info->channel_count++;
        }

		/* skip 20 or 36 bits, depending on crc_present flag */
		if (!crc_present) {
			temp = B_GET_BITS(data, 0, 0) << 2;
			data = batom_cursor_uint32_be(cursor);
			temp |= B_GET_BITS(data, 31, 30);
		}
		else {
			data = batom_cursor_uint32_be(cursor);	
			temp = B_GET_BITS(data, 16, 14);	
		}

		info->sample_size = b_dts_sample_size[temp];
		if (!info->sample_size) {
			return 0;
		}

		return frmsize;
	}

	return 0;
}

static size_t
b_dts_probe_parse_substream_header(const batom_cursor *cursor)
{
    batom_bitstream bs;
    batom_cursor data;
    unsigned nuExtSSHeaderSize = 0;
    unsigned nuExtSSFsize = 0;

    BSTD_UNUSED(nuExtSSHeaderSize);
    BATOM_CLONE(&data, cursor);
    batom_bitstream_init(&bs,&data);
    
    /* UserDefinedBits (8bits) */
    batom_bitstream_drop_bits(&bs,8);

    /*nExtEssIndex (2bits) */
    batom_bitstream_drop_bits(&bs,2);
    
    /* bHeaderSizeType (1bit) */    
    if(batom_bitstream_bit(&bs) == 0) {
        /*nubits4Header (8bits) */
        nuExtSSHeaderSize = (batom_bitstream_bits(&bs,8)+1);
        /*nubits4ExSSFsize (16bits) */
        nuExtSSFsize = (batom_bitstream_bits(&bs,16)+1);
    } else {
        /*nubits4Header (12bits) */
        nuExtSSHeaderSize = (batom_bitstream_bits(&bs,12)+1);
        /*nubits4ExSSFsize (20bits) */
        nuExtSSFsize = (batom_bitstream_bits(&bs,20)+1);
    }
    if(batom_bitstream_eof(&bs) || nuExtSSFsize < 4) {
        return 0;
    }
    
    /* subtract SYNC WORD */
    return nuExtSSFsize-4;  
}

static bmedia_probe_track *
b_dts_probe_feed(bmedia_probe_base_es_t probe, batom_t atom, bool *done)
{
	bdts_probe_t dts = (bdts_probe_t)probe;
	batom_cursor cursor;
	batom_accum_t acc;
	bmedia_probe_audio info;
    
	BSTD_UNUSED(done);

	BDBG_OBJECT_ASSERT(dts, bdts_probe_t);
	acc = dts->acc;
	batom_accum_add_atom(acc, atom);

	BKNI_Memset(&info, 0, sizeof(info)); 
	
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
            temp = ((temp&0xFFFFFF)<<8)|byte;
            if(temp==B_DTS_SYNC) {
                break;
            }
        }

        /* found sync word, now look for back-to-back frames */
        for(frame_cnt=0;frame_cnt<B_DTS_N_FRAMES;frame_cnt++) {
            size_t frame_size = 0;
            BDBG_MSG(("b_dts_probe_feed: %p sync_word at:%u", (void *)dts, (unsigned)batom_cursor_pos(&cursor)-B_DTS_SYNC_LEN));
            if (temp==B_DTS_SYNC) {
                frame_size = b_dts_probe_parse_header(&cursor,&info);
            } else if (temp==B_DTS_SUBSTREAM_SYNC) {
                frame_size = b_dts_probe_parse_substream_header(&cursor);
            }

            if(BATOM_IS_EOF(&cursor)) {
                goto done; /* wait for more data */
            } else if(frame_size>0) {
                BDBG_MSG(("b_dts_probe_feed: %p header %u at:%u size:%u samplerate:%u bitrate:%u", (void *)dts, frame_cnt, (unsigned)batom_cursor_pos(&cursor), (unsigned)frame_size, info.sample_rate, info.bitrate));
                batom_cursor_skip(&cursor, frame_size);
                for(;;) { /* skip trailing zero's */
                    temp = batom_cursor_byte(&cursor);
                    if(BATOM_IS_EOF(&cursor)) {
                        break;
                    }
                    if(temp!=0x00) {
                        break;
                    }
                    BDBG_MSG(("b_dts_probe_feed: %p dropping trailing zero", (void *)dts));
                }
                temp = temp<<24 | batom_cursor_uint24_be(&cursor);
                if(BATOM_IS_EOF(&cursor)) {
                    goto done; /* wait for more data */
                }
                if(temp==B_DTS_SYNC || temp==B_DTS_SUBSTREAM_SYNC) {
                    continue;
                }
                BDBG_MSG(("b_dts_probe_feed: %p out of sync at %u %#x(%#x)", (void *)dts, (unsigned)batom_cursor_pos(&cursor), temp, B_DTS_SYNC));
            }
            byte_cnt+=B_DTS_SYNC_LEN;
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
		BDBG_MSG(("b_dts_probe_feed: %p trim %u", (void *)dts, (unsigned)byte_cnt));
		if(byte_cnt<=B_DTS_SYNC_LEN) {
			break;
		}
		batom_cursor_from_accum(&cursor, acc);
		batom_cursor_skip(&cursor, byte_cnt-B_DTS_SYNC_LEN);
		batom_accum_trim(acc, &cursor);
	}
	return NULL;
}

static const bmedia_probe_file_ext b_dts_ext[] =  {
	{"dts"},
	{""}
};

const bmedia_probe_es_desc bdts_probe = {
	bmedia_track_type_audio,
	{
		baudio_format_dts
	},
	b_dts_ext,
	(2*B_DTS_N_FRAMES)*1920,
	b_dts_probe_create,
	b_dts_probe_destroy,
	b_dts_probe_feed,
	b_dts_probe_reset,
	bmedia_probe_es_nolast
};
