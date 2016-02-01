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
#include "bmpeg_audio_util.h"
#include "biobits.h"
#include "bkni.h"
BDBG_MODULE(bmpeg_audio_util);

static const uint16_t b_mpeg_audio_sample_rate_v1[]={44100, 48000, 32000};
static const uint16_t b_mpeg_audio_sample_rate_v2[]={22050, 24000, 16000};
static const uint16_t b_mpeg_audio_sample_rate_v25[]={11025, 12000, 8000};

static const uint16_t b_mpeg_audio_bitrate_v1_l1[] = {
	/* 0001	*/ 32,	
	/* 0010	*/ 64,	
	/* 0011	*/ 96,	
	/* 0100	*/ 128,	
	/* 0101	*/ 160,	
	/* 0110	*/ 192,	
	/* 0111	*/ 224,	
	/* 1000	*/ 256,	
	/* 1001	*/ 288,	
	/* 1010	*/ 320,	
	/* 1011	*/ 352,	
	/* 1100	*/ 384,	
	/* 1101	*/ 416,	
	/* 1110	*/ 448	
};

static const uint16_t b_mpeg_audio_bitrate_v1_l2[] = {
	/* 0001	*/ 32,		
	/* 0010	*/ 48,		
	/* 0011	*/ 56,		
	/* 0100	*/ 64,		
	/* 0101	*/ 80,		
	/* 0110	*/ 96,
	/* 0111	*/ 112,		
	/* 1000	*/ 128,		
	/* 1001	*/ 160,		
	/* 1010	*/ 192,		
	/* 1011	*/ 224,		
	/* 1100	*/ 256,		
	/* 1101	*/ 320,		
	/* 1110	*/ 384
};

static const uint16_t b_mpeg_audio_bitrate_v1_l3[] = {
	/* 0001	*/ 32,
	/* 0010	*/ 40,		
	/* 0011	*/ 48,		
	/* 0100	*/ 56,		
	/* 0101	*/ 64,		
	/* 0110	*/ 80,
	/* 0111	*/ 96,
	/* 1000	*/ 112,		
	/* 1001	*/ 128,		
	/* 1010	*/ 160,		
	/* 1011	*/ 192,		
	/* 1100	*/ 224,		
	/* 1101	*/ 256,		
	/* 1110	*/ 320
};

static const uint16_t b_mpeg_audio_bitrate_v2_l1[] = {
	/* 0001	*/ 32,	
	/* 0010	*/ 48,
	/* 0011	*/ 56,		
	/* 0100	*/ 64,		
	/* 0101	*/ 80,		
	/* 0110	*/ 96,
	/* 0111	*/ 112,		
	/* 1000	*/ 128,		
	/* 1001	*/ 144,		
	/* 1010	*/ 160,		
	/* 1011	*/ 176,		
	/* 1100	*/ 192,		
	/* 1101	*/ 224,		
	/* 1110	*/ 256
};

static const uint16_t b_mpeg_audio_bitrate_v2_l2l3[] = {
	/* 0001	*/ 8,	
	/* 0010	*/ 16,	
	/* 0011	*/ 24,	
	/* 0100	*/ 32,	
	/* 0101	*/ 40,	
	/* 0110	*/ 48,
	/* 0111	*/ 56,	
	/* 1000	*/ 64,	
	/* 1001	*/ 80,	
	/* 1010	*/ 96,	
	/* 1011	*/ 112,	
	/* 1100	*/ 128,	
	/* 1101	*/ 144,	
	/* 1110	*/ 160
};

#define FRAME_TAG(c1,c2,c3,c4) (((uint32_t)c4) | ((uint32_t)c3<<8) | ((uint32_t)c2<<16) | ((uint32_t)c1<<24))

#define XING_FLAG_FRAMES_PRESENT  (0x01)
#define XING_FLAG_BYTES_PRESENT   (0x02)
#define XING_FLAG_TOC_PRESENT     (0x04)
#define XING_FLAG_QUALITY_PRESENT (0x08)

void
bmp3_parse_vbr_frame_info(batom_cursor *cursor, const bmedia_mpeg_audio_info *mpeg_info, bmp3_vbr_frame_info *vbr_info)
{
    uint32_t flags;
    uint32_t tag;
    size_t header_offset;
    batom_checkpoint ck;

    BDBG_ASSERT(mpeg_info);
    BDBG_ASSERT(vbr_info);

    BKNI_Memset(vbr_info, 0, sizeof(*vbr_info));
    vbr_info->stream_type = bmp3_stream_type_unknown;
    BATOM_SAVE(cursor,&ck);

	if(mpeg_info->version==B_MPEG_AUDIO_VERSION_1) {
        header_offset = (mpeg_info->channel_mode == B_MPEG_AUDIO_CHANNEL_MODE_MONO) ? 17 : 32; /* Mpeg 1: 17 bytes in mono, 32 bytes in stereo */
	} else {
        header_offset = (mpeg_info->channel_mode == B_MPEG_AUDIO_CHANNEL_MODE_MONO) ? 9 : 17;  /* Mpeg 2: 9 bytes in mono, 17 bytes in stereo */
	}
    batom_cursor_skip(cursor, header_offset);
    tag = batom_cursor_uint32_be(cursor);
    if (!BATOM_IS_EOF(cursor) && (tag == FRAME_TAG('X','i','n','g') || tag == FRAME_TAG('I','n','f','o'))) {
        if (tag == FRAME_TAG('X','i','n','g')) {
            vbr_info->stream_type = bmp3_stream_type_vbr;
        } else {
            vbr_info->stream_type = bmp3_stream_type_cbr;
        }

        flags = batom_cursor_uint32_be(cursor);
        if(BATOM_IS_EOF(cursor)) {
            goto exit;
        }

        if (flags & XING_FLAG_FRAMES_PRESENT) {
            vbr_info->num_frames = batom_cursor_uint32_be(cursor);
        }
        if (flags & XING_FLAG_BYTES_PRESENT) {
            vbr_info->stream_bytes = batom_cursor_uint32_be(cursor);
        }
        if (flags & XING_FLAG_TOC_PRESENT) {
            batom_cursor_skip(cursor, 100); /* Skip the TOC */
        }
        if (flags & XING_FLAG_QUALITY_PRESENT) {
            vbr_info->stream_quality = batom_cursor_uint32_be(cursor); /* 100 is the maximum value */
        }
    } else {
        /* Check for VBRI header which always is located at byte offset 32 */
        batom_cursor_rollback(cursor, &ck);
        batom_cursor_skip(cursor, 32);
        tag = batom_cursor_uint32_be(cursor);
        if(!BATOM_IS_EOF(cursor) && tag == FRAME_TAG('V','B','R','I')) {
            vbr_info->stream_type = bmp3_stream_type_vbr;
            batom_cursor_skip(cursor, 2); /* Version */
            batom_cursor_skip(cursor, 2); /* Delay */
            vbr_info->stream_quality = batom_cursor_uint16_be(cursor); /* 100 is the maximum value */
            vbr_info->stream_bytes = batom_cursor_uint32_be(cursor);
            vbr_info->num_frames = batom_cursor_uint32_be(cursor);
        } else {
            goto exit;
        }
    }
    if(BATOM_IS_EOF(cursor)) {
        goto exit;
    }
    return;

exit:
    return;
}

size_t
bmpeg_audio_probe_parse_header(batom_cursor *cursor, uint16_t sync_word, bmedia_probe_audio *info, bmedia_mpeg_audio_info *mpeg_info)
{
	unsigned version = B_GET_BITS(sync_word, 4, 3);
	unsigned layer = B_GET_BITS(sync_word, 2, 1);

	BDBG_MSG(("b_mpeg_audio_probe_parse_header: version:%u layer:%u", version, layer));
	if(version!=0x01 && layer!=0x00) {
		const uint16_t *bitrate_table;
		unsigned index;
        sync_word = batom_cursor_uint16_be(cursor);
        mpeg_info->layer = layer;
        mpeg_info->version = version;
        mpeg_info->channel_mode = B_GET_BITS(sync_word, 7,6);
		if(version==B_MPEG_AUDIO_VERSION_1) {
			if(layer==B_MPEG_AUDIO_LAYER_I) {
				bitrate_table = b_mpeg_audio_bitrate_v1_l1;
			} else if (layer==B_MPEG_AUDIO_LAYER_II) {
				bitrate_table = b_mpeg_audio_bitrate_v1_l2;
			} else {
				bitrate_table = b_mpeg_audio_bitrate_v1_l3;
			}
		} else {
			if(layer==B_MPEG_AUDIO_LAYER_I) {
				bitrate_table = b_mpeg_audio_bitrate_v2_l1;
			} else {
				bitrate_table = b_mpeg_audio_bitrate_v2_l2l3;
			}
		}
		index = B_GET_BITS(sync_word, 15, 12)-1;
		if(index<sizeof(b_mpeg_audio_bitrate_v1_l1)/sizeof(*b_mpeg_audio_bitrate_v1_l1)) {
			info->bitrate = bitrate_table[index];
			if(layer!=B_MPEG_AUDIO_LAYER_III) {
				info->codec = baudio_format_mpeg;
			} else {
				info->codec = baudio_format_mp3;
			}
			index = B_GET_BITS(sync_word, 11, 10);
			if(index<sizeof(b_mpeg_audio_sample_rate_v1)/sizeof(*b_mpeg_audio_sample_rate_v1)) {
				unsigned padding = B_GET_BITS(sync_word, 9,9);
				unsigned frame_len;
                unsigned scale;
				if(version==B_MPEG_AUDIO_VERSION_1) {
					info->sample_rate = b_mpeg_audio_sample_rate_v1[index];
                    scale = 0;
				} else if(version==B_MPEG_AUDIO_VERSION_2) {
					info->sample_rate = b_mpeg_audio_sample_rate_v2[index];
                    scale = layer==B_MPEG_AUDIO_LAYER_III ? 1 : 0;
				} else {
					info->sample_rate = b_mpeg_audio_sample_rate_v25[index];
                    scale = 1;
				}
				info->sample_size = 16;
                info->channel_count = (mpeg_info->channel_mode == B_MPEG_AUDIO_CHANNEL_MODE_MONO) ? 1 : 2;
				if(layer==B_MPEG_AUDIO_LAYER_I) {
                    mpeg_info->samples = (384>>scale);
					frame_len = (((12000*info->bitrate)/(info->sample_rate<<scale))+padding)*4;
				} else {
                    mpeg_info->samples = 1152>>scale;
                    frame_len = (((1152/8)*1000*info->bitrate)/(info->sample_rate<<scale))+padding;
                }
				return frame_len - B_MPEG_AUDIO_HEADER;
			}
		}
	}
	return 0;
}


