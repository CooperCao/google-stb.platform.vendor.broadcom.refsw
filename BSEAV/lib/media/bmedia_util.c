/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2007-2016 Broadcom.  All rights reserved.
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
 * BMedia library, misc utilities
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bmedia_util.h"
#include "biobits.h"
#include "bkni.h"

BDBG_MODULE(bmedia_util);

#define BDBG_MSG_TRACE(x)   /* BDBG_MSG(x) */


#define B_MEDIA_PTS_SCALE (BMPEG2PES_PTS_UNITS/1000)
uint32_t
bmedia_pts2time(uint32_t pts, bmedia_time_scale scale)
{
	uint64_t time;

	if (scale>=0) {
		time = ((uint64_t)pts)*scale/(B_MEDIA_PTS_SCALE*BMEDIA_TIME_SCALE_BASE);
		return (uint32_t)time;
	} else {
		pts = (~pts)&(uint32_t)(BMEDIA_PTS_MODULO-1);
		time = ((uint64_t)pts)*(-scale)/(B_MEDIA_PTS_SCALE*BMEDIA_TIME_SCALE_BASE);
		return (uint32_t)(time);
	}
}

uint32_t
bmedia_time2pts(uint32_t time, bmedia_time_scale scale)
{
	uint64_t pts;

	BDBG_ASSERT(scale);
	if (scale>=0) {
		pts = (((uint64_t)time)*BMEDIA_TIME_SCALE_BASE*B_MEDIA_PTS_SCALE)/scale;
		return (uint32_t)(pts&(BMEDIA_PTS_MODULO-1));
	} else {
		pts = (((uint64_t)time)*BMEDIA_TIME_SCALE_BASE*B_MEDIA_PTS_SCALE)/(-scale);
		return (~(uint32_t)(pts))&(uint32_t)(BMEDIA_PTS_MODULO-1);
	}
}

static void
b_media_adts_header_aac_init(const bmedia_info_aac *aac, bmedia_info_aac *initialized_aac)
{
   initialized_aac->channel_configuration = aac->channel_configuration;

   switch(aac->profile)
   {
    /* AAC Main */
    case 1:
        initialized_aac->profile = 0;
        break;
    /* AAC LC and AAC SBR */
    case 2:
    case 5:
        initialized_aac->profile = 1;
        break;
    /* AAC SSR */
    case 3:
        initialized_aac->profile = 2;
        break;
    /*AAC LTP */
    case 4:
        initialized_aac->profile = 3;
        break;
    /* There are other types of Audio object types, but non fit in to any of the ADTS profiles.
       There has been no test content for the other types.  Defaulting to AAC LC */
    default:
        initialized_aac->profile = 1;
   }

   /* If the stream is SBR we need to program the ADTS header with the AAC-LC sample rate index */
   if (aac->profile == 5)
   {
        size_t sample_rate;

        sample_rate = bmedia_info_aac_sampling_frequency_from_index(aac->sampling_frequency_index);
        sample_rate = sample_rate / 2;
        if(!(bmedia_info_aac_set_sampling_frequency_index(initialized_aac,sample_rate)))
        {
            initialized_aac->sampling_frequency_index = aac->sampling_frequency_index;
        }
   }
   else
   {
       initialized_aac->sampling_frequency_index = aac->sampling_frequency_index;
   }

}

void
bmedia_adts_header_init_aac(bmedia_adts_header *header, const bmedia_info_aac *aac)
{
    bmedia_info_aac initialized_aac;
    BDBG_ASSERT(header);
    BDBG_ASSERT(aac);
    b_media_adts_header_aac_init(aac, &initialized_aac);
    BDBG_MSG_TRACE(("bmedia_adts_header_init_aac: profile:%u sampling_frequency_index:%u channel_configuration:%u", (unsigned)initialized_aac.profile, (unsigned)initialized_aac.sampling_frequency_index, (unsigned)initialized_aac.channel_configuration));
    /* ISO 13818-7  1.2.1    Fixed Header of ADTS  */
    header->adts[0] =
        0xFF; /* sync word */
    header->adts[1] =
        0xF0 |  /* sync word */
        B_SET_BIT( ID, 0, 3) | /* MPEG-2 AAC */
        B_SET_BITS( "00", 0, 2, 1) |
        B_SET_BIT( protection_absent, 1, 0);

    header->adts[2] =
        B_SET_BITS( profile, initialized_aac.profile, 7, 6) |
        B_SET_BITS( sampling_frequency_index, initialized_aac.sampling_frequency_index, 5, 2 ) |
        B_SET_BIT( private_bit, 0, 1) |
        B_SET_BIT( channel_configuration[2], B_GET_BIT(initialized_aac.channel_configuration, 2), 0);

    header->adts_3 = /* 4'th byte is shared */
        B_SET_BITS( channel_configuration[2], B_GET_BITS(initialized_aac.channel_configuration, 1, 0), 7, 6) |
        B_SET_BIT( original_copy, 0, 5) |
        B_SET_BIT( home, 0, 4) |
        /* IS 13818-7 1.2.2	Variable Header of ADTS */
        B_SET_BIT( copyright_identification_bit, 0, 3) |
        B_SET_BIT( copyright_identification_start, 0, 2);
    /* bytes 5,6 and 7 depend on the frame length */
    return;
}

size_t
bmedia_adts_header_fill(uint8_t *hdr_buf, const bmedia_adts_header *header, size_t frame_length)
{
    BDBG_ASSERT(hdr_buf);
    BDBG_ASSERT(header);

    frame_length += BMEDIA_ADTS_HEADER_SIZE;

    /* ISO 13818-7  1.2.1    variable header of ADTS  */
    hdr_buf[0] = header->adts[0];
    hdr_buf[1] = header->adts[1];
    hdr_buf[2] = header->adts[2];
    hdr_buf[3] = header->adts_3 |
            B_SET_BITS(aac_frame_length[12..11], B_GET_BITS(frame_length, 12, 11), 1, 0);
    hdr_buf[4] =
            B_SET_BITS(aac_frame_length[10..3], B_GET_BITS(frame_length, 10, 3), 7, 0);
    hdr_buf[5] =
            B_SET_BITS(aac_frame_length[2..0], B_GET_BITS(frame_length, 2, 0), 7, 5) |
            B_SET_BITS(adts_buffer_fullness[10..6], B_GET_BITS( 0x7FF /* VBR */, 10, 6), 4, 0);
    hdr_buf[6] =
            B_SET_BITS(adts_buffer_fullness[5..0], B_GET_BITS( 0x7FF /* VBR */, 5, 0), 7, 2) |
            B_SET_BITS(no_raw_data_blocks_in_frame, 0, 2, 0);

    return BMEDIA_ADTS_HEADER_SIZE;
}

void
bmedia_pes_info_init(bmedia_pes_info *info, uint8_t id)
{
    BDBG_ASSERT(info);
    info->stream_id = id;
    info->dts_valid = false;
    info->pts_valid = false;
    info->pts = 0;
    info->dts = 0;
    return;
}

size_t
bmedia_pes_header_init(uint8_t *pes_header, size_t length, const bmedia_pes_info *info)
{
	unsigned off;
	uint8_t ptd_dts_flag;

    BDBG_ASSERT(info);
    BDBG_MSG(("bmedia_pes_header_init>: %p %#x %u %s %u", (void *)pes_header, info->stream_id, (unsigned)length, info->pts_valid?"PTS":"NO-PTS", info->pts_valid?info->pts:0));

	pes_header[0] = 0;
	pes_header[1] = 0;
	pes_header[2] = 1;
	pes_header[3] = info->stream_id;

    if(info->pts_valid) {
    	if(info->dts_valid){
		    ptd_dts_flag = 0x3;	
	    } else {
		    ptd_dts_flag = 0x2;	
	    }
    } else {
        ptd_dts_flag = 0x0;
    }

	pes_header[6] = 
		B_SET_BITS( "10", 2, 7, 6 ) |
		B_SET_BITS(PES_scrambling_control, 0, 5, 4) |
		B_SET_BIT(PES_priority, 0, 3) |
		B_SET_BIT(data_alignment_indicator,/*frame_boundary*/ false, 2) |
		B_SET_BIT(copyright, 0, 1) |
		B_SET_BIT(original_or_copy, 1, 0);
	pes_header[7] = 
		B_SET_BITS(PTS_DTS_flags, ptd_dts_flag, 7, 6) |
		B_SET_BIT(ESCR_flag, 0, 5) |
		B_SET_BIT(ES_rate_flag, 0, 4) |
		B_SET_BIT(DSM_trick_mode_flag, 0, 3) |
		B_SET_BIT(additional_copy_info_flag, 0, 2) |
		B_SET_BIT(PES_CRC_flag, 0, 1) |
		B_SET_BIT(PES_extension_flag, false, 0);
	if (info->pts_valid) {	
		pes_header[9] = 
			B_SET_BITS("0010", ptd_dts_flag, 7, 4) |
			B_SET_BITS("PTS [32..30]", B_GET_BITS(info->pts,31,29), 3, 1) |
			B_SET_BIT(marker_bit, 1, 0);
		pes_header[10] = B_GET_BITS(info->pts,28,21); /* PTS [29..15] -> PTS [29..22] */
		pes_header[11] = 
			B_SET_BITS("PTS [29..15] -> PTS [21..15]", B_GET_BITS(info->pts,20,14), 7, 1) |
			B_SET_BIT(marker_bit, 1, 0);
		pes_header[12] = B_GET_BITS(info->pts,13,6); /* PTS [14..0] -> PTS [14..7]  */
		pes_header[13] = 
			B_SET_BITS("PTS [14..0] -> PTS [6..0]", B_GET_BITS(info->pts,5,0), 7, 2) |
			B_SET_BIT("PTS[0]", 0, 1) |
			B_SET_BIT(marker_bit, 1, 0);

		off = 14;

		if(info->dts_valid){
			pes_header[14] = 
			B_SET_BITS("0001", 0x1, 7, 4) |
			B_SET_BITS("DTS [32..30]", B_GET_BITS(info->dts,31,29), 3, 1) |
			B_SET_BIT(marker_bit, 1, 0);
			pes_header[15] = B_GET_BITS(info->dts,28,21); /* DTS [29..15] -> DTS [29..22] */
			pes_header[16] = 
				B_SET_BITS("DTS [29..15] -> DTS [21..15]", B_GET_BITS(info->dts,20,14), 7, 1) |
				B_SET_BIT(marker_bit, 1, 0);
			pes_header[17] = B_GET_BITS(info->dts,13,6); /* DTS [14..0] -> DTS [14..7]  */
			pes_header[18] = 
				B_SET_BITS("DTS [14..0] -> DTS [6..0]", B_GET_BITS(info->dts,5,0), 7, 2) |
				B_SET_BIT("DTS[0]", 0, 1) |
				B_SET_BIT(marker_bit, 1, 0);

			off += 5;
		}
	} else {
		off = 9;
	}

	pes_header[8] = off - 9;
	/* length */
	if (length>0) {
		length += (off-6);
		if (length>65536) {
            if((info->stream_id&0xF0)!=0xE0) { /* if not video ID */
                BDBG_WRN(("bmedia_pes_header_init: unbounded pes %02x %u", info->stream_id, (unsigned)length));
            }
            length = 0;
		}
	}
	pes_header[4] = B_GET_BITS(length, 15, 8);
	pes_header[5] = B_GET_BITS(length, 7, 0);

    BDBG_ASSERT(off<BMEDIA_PES_HEADER_MAX_SIZE);

    BDBG_MSG_TRACE(("bmedia_pes_header_init<: %p %#x %u %u", (void *)pes_header, info->stream_id, (unsigned)length, off));

    return off;
}

/* ISO/IEC 13818-7:2005(E) */
/* Table 35 . Sampling frequency dependent on sampling_frequency_index. Must remain sorted in decending order */
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


bool 
bmedia_info_aac_set_sampling_frequency_index(bmedia_info_aac *aac, unsigned sampling_frequency)
{
	unsigned i, dif;
	BDBG_ASSERT(aac);
	for(i=0;i<sizeof(b_aac_adts_sample_rate)/sizeof(*b_aac_adts_sample_rate);i++) {
        if (b_aac_adts_sample_rate[i] == sampling_frequency) {
            aac->sampling_frequency_index = i;
            return true;
        } else 
            if(b_aac_adts_sample_rate[i] < sampling_frequency) {
                break;
		}
	}

    /* Find the closest samplerate in the list to the desired sample rate.
       We use the previous sample rate index if:
          1. The provided sample rate is less than any on the list (i == num_sample_rates)
          2. The provided sample rate is not larger than any on the list (i>0) *and*
             The provided sample rate is closer to the previous list rate than it is to the current list rate */
    if ((i == sizeof(b_aac_adts_sample_rate)/sizeof(*b_aac_adts_sample_rate)) || 
        (i>0 && ((sampling_frequency - b_aac_adts_sample_rate[i]) > (b_aac_adts_sample_rate[i-1] - sampling_frequency)))){
        i--;
    }

    if(sampling_frequency > 0) {
        /* Allow for a successful hit with a .5% margin of error */
        dif = (b_aac_adts_sample_rate[i] * 1000) / sampling_frequency;
        if ((dif >= 995) && (dif <= 1005)) {
            aac->sampling_frequency_index = i;
            return true;
        }
    }
	BDBG_WRN(("bmedia_info_aac_set_sampling_frequency_index: unknown frequency %u", sampling_frequency));
	return false;
}

unsigned
bmedia_info_aac_sampling_frequency_from_index(unsigned index) {
    if (index < sizeof(b_aac_adts_sample_rate)/sizeof(*b_aac_adts_sample_rate)) {
        return b_aac_adts_sample_rate[index];
    }
    return 0;
}


bool 
bmedia_info_aac_set_channel_configuration(bmedia_info_aac *aac, unsigned nchannels)
{
    unsigned i;
	BDBG_ASSERT(aac);
	for(i=0;i<sizeof(b_aac_adts_channels)/sizeof(*b_aac_adts_channels);i++) {
		if(b_aac_adts_channels[i]==nchannels) {
			aac->channel_configuration = i;
			return true;
		}
	}
	BDBG_WRN(("bmedia_info_aac_set_channel_configuration: unknown number of channels %u", nchannels));
	return false;
}

/* size of the basic WAVEHEADER, without cbSize field */
#define B_MEDIA_WAV_HEADER_SIZE (BMEDIA_FIELD_LEN(wFormatTag, uint16_t) + BMEDIA_FIELD_LEN(nChannels, uint16_t) + BMEDIA_FIELD_LEN(nSamplesPerSec, uint32_t) + BMEDIA_FIELD_LEN(nAvgBytesPerSec, uint32_t) + BMEDIA_FIELD_LEN(nBlockAlign, uint16_t) + BMEDIA_FIELD_LEN(wBitsPerSample, uint16_t))

bool
bmedia_read_waveformatex(bmedia_waveformatex *wf, batom_cursor *c)
{
    batom_cursor waveformat;
    size_t header_len = B_MEDIA_WAV_HEADER_SIZE;

    BDBG_ASSERT(sizeof(wf->meta)>=header_len+BMEDIA_FIELD_LEN(cbSize, uint16_t));

    BATOM_CLONE(&waveformat, c);

	wf->wFormatTag = batom_cursor_uint16_le(c);
	wf->nChannels = batom_cursor_uint16_le(c);
	wf->nSamplesPerSec = batom_cursor_uint32_le(c);
	wf->nAvgBytesPerSec = batom_cursor_uint32_le(c);
	wf->nBlockAlign = batom_cursor_uint16_le(c);
	wf->wBitsPerSample = batom_cursor_uint16_le(c);
	if(BATOM_IS_EOF(c)) {
        BDBG_ERR(("%s: %p incomplete header", "bmedia_waveformatex", (void *)wf));
        return false;
    }
    wf->cbSize = batom_cursor_uint16_le(c);
    if(!BATOM_IS_EOF(c)) {
        size_t meta_length;
        header_len  += BMEDIA_FIELD_LEN(cbSize, uint16_t) + wf->cbSize;
        meta_length = batom_cursor_reserve(&waveformat, header_len);
        if(meta_length != header_len) {
            BDBG_WRN(("%s: cbSize %u exceeds size of the waveformat data %u", "bmedia_waveformatex", (unsigned)header_len, (unsigned)meta_length));
        } else if(header_len > sizeof(wf->meta)) {
            BDBG_ERR(("%s: WAVE header %u exceeds reserved size %u", "bmedia_waveformatex", (unsigned)header_len, (unsigned)sizeof(wf->meta)));
        } else {
            meta_length = batom_cursor_copy(&waveformat, wf->meta, header_len);
            BDBG_ASSERT(header_len==meta_length);
            goto done;
        }
    } else {
        BDBG_WRN(("%s: %px missing cbSize", "bmedia_waveformatex", (void *)wf));
    }
    /* special case where size field is not present  and/or invalid */
    header_len = B_MEDIA_WAV_HEADER_SIZE;
    wf->cbSize =  0;
    batom_cursor_copy(&waveformat, wf->meta, header_len);
    B_MEDIA_SAVE_UINT16_LE(wf->meta + header_len, 0);
    header_len += BMEDIA_FIELD_LEN(cbSize, uint16_t);

done:
    wf->meta_length = header_len;

    BDBG_MSG(("%s: FormatTag:%#x Channels:%u SamplesPerSec:%u AvgBytesPerSec:%u nBlockAlign:%u BitsPerSample:%u Size:%u", "bmedia_waveformatex", wf->wFormatTag, wf->nChannels, wf->nSamplesPerSec, wf->nAvgBytesPerSec, wf->nBlockAlign, wf->wBitsPerSample, wf->cbSize));
    return true;
}


size_t
bmedia_copy_waveformatex(const bmedia_waveformatex *wf, void *buf, size_t max_length)
{
    BDBG_ASSERT(wf);
    BDBG_ASSERT(buf);

    if(max_length >= wf->meta_length) {
        BKNI_Memcpy(buf, wf->meta, wf->meta_length); 
        return wf->meta_length;
    } else {
        BKNI_Memcpy(buf, wf->meta, max_length); 
        if(max_length >= BMEDIA_WAVEFORMATEX_BASE_SIZE) {
            /* correct embedded chSize */
            unsigned cbSize = max_length - BMEDIA_WAVEFORMATEX_BASE_SIZE;
            BDBG_CASSERT(B_MEDIA_WAV_HEADER_SIZE < BMEDIA_WAVEFORMATEX_BASE_SIZE); 
            ((uint8_t *)buf)[B_MEDIA_WAV_HEADER_SIZE] = B_GET_BITS(cbSize, 7, 0);
            ((uint8_t *)buf)[B_MEDIA_WAV_HEADER_SIZE+1] = B_GET_BITS(cbSize, 15, 8);
        }
        return max_length;
    }
}

void bmedia_init_waveformatex(bmedia_waveformatex_header *wf)
{
    BKNI_Memset(wf,0,sizeof(*wf));
    wf->nChannels = 1;
    wf->nSamplesPerSec = 32000;
    wf->nAvgBytesPerSec = 32000;
    wf->nBlockAlign = 1;
    wf->wBitsPerSample = 8;
    return;
}

size_t bmedia_write_waveformatex(void *buf, const bmedia_waveformatex_header *wf)
{
    uint8_t *waveheader = buf;
    B_MEDIA_SAVE_UINT16_LE(waveheader+0, wf->wFormatTag);
    B_MEDIA_SAVE_UINT16_LE(waveheader+2, wf->nChannels);
    B_MEDIA_SAVE_UINT32_LE(waveheader+4, wf->nSamplesPerSec);
    B_MEDIA_SAVE_UINT32_LE(waveheader+8, wf->nAvgBytesPerSec);
    B_MEDIA_SAVE_UINT16_LE(waveheader+12, wf->nBlockAlign);
    B_MEDIA_SAVE_UINT16_LE(waveheader+14, wf->wBitsPerSample);
    B_MEDIA_SAVE_UINT16_LE(waveheader+16, wf->cbSize);
    return BMEDIA_WAVEFORMATEX_BASE_SIZE;
}

void bmedia_waveformatex_from_header(bmedia_waveformatex *wf, bmedia_waveformatex_header *header)
{
    BKNI_Memset(wf, 0, sizeof(*wf));
    wf->wFormatTag = header->wFormatTag;
    wf->nChannels = header->nChannels;
    wf->nSamplesPerSec = header->nSamplesPerSec;
    wf->nAvgBytesPerSec = header->nAvgBytesPerSec;
    wf->nBlockAlign = header->nBlockAlign;
    wf->wBitsPerSample = header->wBitsPerSample;
    wf->cbSize = header->cbSize;
    return;
}
bool
bmedia_read_bitmapinfo(bmedia_bitmapinfo *bi, batom_cursor *c)
{
	bi->biSize = batom_cursor_uint32_le(c);
	bi->biWidth = (int32_t)batom_cursor_uint32_le(c);
	bi->biHeight =  (int32_t)batom_cursor_uint32_le(c);
	bi->biPlanes = batom_cursor_uint16_le(c);
	bi->biBitCount = batom_cursor_uint16_le(c);
	bi->biCompression = batom_cursor_uint32_le(c);
	bi->biSizeImage = batom_cursor_uint32_le(c);
	bi->biXPelsPerMeter = (int32_t)batom_cursor_uint32_le(c);
	bi->biYPelsPerMeter = (int32_t)batom_cursor_uint32_le(c);
	bi->biClrUsed = (int32_t)batom_cursor_uint32_le(c);
	bi->biClrImportant = (int32_t)batom_cursor_uint32_le(c);
	BDBG_MSG(("bmedia_read_bitmapinfo: Size:%u (%ux%u) Compression:" BMEDIA_FOURCC_FORMAT  , (unsigned)bi->biSize, (unsigned)bi->biWidth, (unsigned)bi->biHeight, BMEDIA_FOURCC_ARG(bi->biCompression)));

	return !BATOM_IS_EOF(c);
}

bool
bmedia_vc1ap_read_info(bmedia_vc1ap_info *info, batom_cursor *cursor)
{
	int byte;
	batom_cursor_skip(cursor, 9);
	byte = batom_cursor_next(cursor);
	info->interlaced = false;
	if(byte==BATOM_EOF) {
		return false;
	}
	info->interlaced = (byte & 0x40)!=0;
	return true;
}

bool
bmedia_vc1sm_read_info(bmedia_vc1sm_info *info, batom_cursor *cursor)
{
	unsigned profile;
	unsigned frmrtq_postproc;
	unsigned bitrtq_postproc;
	unsigned loopfilter;
	unsigned multires;
	unsigned fastuvmc;
	unsigned extended_mv;
	unsigned dquant;
	unsigned vstransform;
	unsigned overlap;
	unsigned syncmarker;
	unsigned rangered;
	unsigned maxbframes;
	unsigned quantizer;
	unsigned finterpflag;
	batom_bitstream bs;

	batom_bitstream_init(&bs, cursor);

	profile = batom_bitstream_bits(&bs, 4);
	BDBG_MSG(("bmedia_vc1sm_read_info>: header: %#x", bs.cache));

	frmrtq_postproc = batom_bitstream_bits(&bs, 3);
	bitrtq_postproc = batom_bitstream_bits(&bs, 5);
	loopfilter = batom_bitstream_bit(&bs);
	batom_bitstream_drop(&bs); /* Reserved3 */
	multires = batom_bitstream_bit(&bs);
	batom_bitstream_drop(&bs); /* Reserved4 */
	fastuvmc = batom_bitstream_bit(&bs);
	extended_mv = batom_bitstream_bit(&bs);
	dquant = batom_bitstream_bits(&bs, 2);
	vstransform = batom_bitstream_bit(&bs);
	batom_bitstream_drop(&bs); /* Reserved5 */
	overlap = batom_bitstream_bit(&bs);
	syncmarker = batom_bitstream_bit(&bs);
	rangered = batom_bitstream_bit(&bs);
	maxbframes = batom_bitstream_bits(&bs, 3);
	quantizer = batom_bitstream_bits(&bs, 2);
	finterpflag = batom_bitstream_bit(&bs);
	batom_bitstream_drop(&bs); /* Reserved6 */

	/* batom_bitstream_dump(&bs); */
	
	BDBG_MSG(("bmedia_vc1sm_read_info>: profile: %u frmrtq_postproc: %u bitrtq_postproc: %u loopfilter: %u", profile, frmrtq_postproc, bitrtq_postproc, loopfilter));
	BDBG_MSG(("bmedia_vc1sm_read_info>: rangered: %d maxbframes: %d quantizer: %d finterpflag: %d", rangered, maxbframes, quantizer, finterpflag));
	info->finterpflag = finterpflag;
	info->rangered = rangered;
	info->maxbframes = maxbframes;
	return true;
}

typedef enum b_media_video_frame_type {
	b_media_video_frame_type_frame_progressive,
	b_media_video_frame_type_frame_interlaced,
	b_media_video_frame_type_field_interlaced
} b_media_video_frame_type;


bmedia_video_pic_type 
bmedia_vc1ap_read_pic_type(batom_cursor *cursor, const bmedia_vc1ap_info *info)
{
	uint32_t sc;
	int byte;
	b_media_video_frame_type frame_type;
	bmedia_video_pic_type pic_type;

	BATOM_NEXT(byte, cursor);
	if(byte==BIO_EOF) {
		goto eof;
	}
	sc = ((uint32_t)byte<<24) | batom_cursor_uint24_be(cursor);
	if( (sc&0xFFFFFF00) == 0x0100) { /* some start code */
		 while(sc!=0x10D) {
			/* not a picture scartcode, try to load the next one*/
			BATOM_NEXT(byte, cursor);
			if(byte==BIO_EOF) {
				goto eof;
			}
			sc = (sc << 8)|(uint32_t)byte;
		}
		BATOM_NEXT(byte, cursor);
		if(byte==BIO_EOF) {
			goto eof;
		}
	}
	BDBG_MSG_TRACE(("bmedia_vc1ap_read_pic_type>: sc:%#x byte:%#x", (unsigned)sc, (unsigned)byte));
	if(info->interlaced) {
		if( (byte&0x80) == 0) { 
			frame_type = b_media_video_frame_type_frame_progressive; 
			byte <<=1;
		} else if ( (byte&0xC0) == 0x80) {
			frame_type = b_media_video_frame_type_frame_interlaced;
			byte <<=2;
		} else {
			frame_type = b_media_video_frame_type_field_interlaced;
			byte <<=2;
		}
	} else {
		frame_type = b_media_video_frame_type_frame_progressive; 
	}
	/* coverity[dead_error_condition] */
	switch(frame_type) {
	case b_media_video_frame_type_frame_interlaced:
	case b_media_video_frame_type_frame_progressive:
		if((byte&0x80)==0) {
			pic_type = bmedia_video_pic_type_P;
		} else if ((byte&0xC0) == 0x80) {
			pic_type = bmedia_video_pic_type_B;
		} else if ((byte&0xE0) == 0xC0) { 
			pic_type = bmedia_video_pic_type_I;
		} else if ((byte&0xF0) == 0xE0) { 
			pic_type = bmedia_video_pic_type_B;  /* "BI";  */
		} else {
			pic_type = bmedia_video_pic_type_skipped;
		}
		break;
	case b_media_video_frame_type_field_interlaced:
		switch((byte&0xE0)>>5) {
		case 0: pic_type = bmedia_video_pic_type_I; /* "I:I" */ break;
		case 1: pic_type = bmedia_video_pic_type_I; /* "I:P" */ break;
		case 2: pic_type = bmedia_video_pic_type_P; /* "P:I" */ break;
		case 3: pic_type = bmedia_video_pic_type_P; /* "P:P" */ break;
		case 4: pic_type = bmedia_video_pic_type_B; /* "B:B" */ break;
		case 5: pic_type = bmedia_video_pic_type_B; /* "B:BI" */ break;
		case 6: pic_type = bmedia_video_pic_type_B; /* "BI:B" */ break;
		case 7: pic_type = bmedia_video_pic_type_B; /* "BI:BI" */ break;
		default:  pic_type = bmedia_video_pic_type_unknown; break;
		}
		break;
	/* coverity[dead_error_begin] */
	default:
		pic_type = bmedia_video_pic_type_unknown; break;
	}
done:
	BDBG_MSG_TRACE(("bmedia_vc1ap_read_pic_type>: pic_type:%u", (unsigned)pic_type));
	return pic_type;
eof:
	BDBG_MSG_TRACE(("bmedia_vc1ap_read_pic_type: can't find picture header"));
	pic_type = bmedia_video_pic_type_unknown;
	goto done;
}

bmedia_video_pic_type 
bmedia_vc1sm_read_pic_type(batom_cursor *cursor, const bmedia_vc1sm_info *info)
{
	batom_bitstream bs;
	bmedia_video_pic_type pic_type = bmedia_video_pic_type_unknown;

	batom_bitstream_init(&bs, cursor);
	if (info->finterpflag) {
		batom_bitstream_drop(&bs); /* INTERPFRM */
	}
	batom_bitstream_drop_bits(&bs, 2); /* FRMCNT */
	if (info->rangered) {
		batom_bitstream_drop(&bs); /* RANGEREDFRM */
	}
	if (batom_bitstream_bit(&bs)) {
		pic_type = bmedia_video_pic_type_P;
	} else if (info->maxbframes==0) {
		pic_type = bmedia_video_pic_type_I;
	} else if (batom_bitstream_bit(&bs)) {
		pic_type = bmedia_video_pic_type_I;
	} else {
		pic_type = bmedia_video_pic_type_B;
	}
	if(batom_bitstream_eof(&bs)) {
		BDBG_MSG_TRACE(("bmedia_vc1sm_read_pic_type>: %p can't read picture type", (void *)cursor));
		pic_type = bmedia_video_pic_type_unknown;
	}

	BDBG_MSG_TRACE(("bmedia_vc1sm_read_pic_type>: pic_type:%u", (unsigned)pic_type));
	return pic_type;
}

int
bmedia_video_read_scode(batom_cursor *cursor)
{
	uint32_t scode = batom_cursor_uint32_be(cursor);
    if((scode&0xFFFFFF00)==0x100) {  /* this test also covers EOF case, where batom_cursor_uint32_be returns 0xFFFFFFFF */
        return (int)(scode & 0xFF);
    } else {
        return -1;
    }
}

uint32_t
bmedia_video_scan_scode(batom_cursor *cursor, size_t range)
{
    uint32_t scode;
    int byte;
    size_t count;

    for(scode=0xFF,count=0;count<range;count++) {
        BATOM_NEXT(byte, cursor);
        if(byte==BATOM_EOF) {
            break;
        }
        scode = (scode << 8)|byte;
        if((scode&0xFFFFFF00ul)==0x100) {
            return scode;
        }
    }
    return 0;
}


bmedia_video_pic_type
bmpeg4_video_read_pic_type(batom_cursor *cursor)
{
    uint32_t scode;
    bmedia_video_pic_type pic_type = bmedia_video_pic_type_unknown;
    size_t limit_pos = batom_cursor_pos(cursor) + 256; /* only peek in first 256 bytes */
    int byte;

    for(;;) {
        size_t current_pos = batom_cursor_pos(cursor);
        if(current_pos>=limit_pos) {
            goto no_scode;
        }
        scode = bmedia_video_scan_scode(cursor, limit_pos-current_pos);
        if(scode==0) {
            goto no_scode;
        }
        if(scode == 0x01B6) {
            break;
        }
    }

    BATOM_NEXT(byte, cursor);
    if(byte==BATOM_EOF) { goto no_scode;}
    switch(B_GET_BITS(byte, 7, 6)) {
        case 0:
            pic_type = bmedia_video_pic_type_I;
            break;
        case 1:
            pic_type = bmedia_video_pic_type_P;
            break;
        case 2:
            pic_type = bmedia_video_pic_type_B;
            break;
        default:
            break;
    }

no_scode:
    return pic_type;
}


static const uint8_t b_media_eos_h264[] = { 0x00, 0x00, 0x01, 0x0A };
const batom_vec bmedia_eos_h264  = BATOM_VEC_INITIALIZER((void *)b_media_eos_h264, sizeof(b_media_eos_h264));

static const uint8_t b_media_eos_h265[] = { 0x00, 0x00, 0x01, 0x4A, 0x01 };
const batom_vec bmedia_eos_h265= BATOM_VEC_INITIALIZER((void *)b_media_eos_h265, sizeof(b_media_eos_h265));

static const uint8_t b_media_eos_mpeg4[] = { 0x00, 0x00, 0x01, 0xB1, 0x00, 0x00, 0x01, 0xB1 };
const batom_vec bmedia_eos_mpeg4  = BATOM_VEC_INITIALIZER((void *)b_media_eos_mpeg4, sizeof(b_media_eos_mpeg4));

static const uint8_t b_media_frame_mpeg4[] = { 0x00, 0x00, 0x01, 0xB6 };
const batom_vec bmedia_frame_mpeg4 = BATOM_VEC_INITIALIZER((void *)b_media_frame_mpeg4, sizeof(b_media_frame_mpeg4));

static const uint8_t b_media_frame_bcma[] = { 'B', 'C', 'M', 'A' };
const batom_vec bmedia_frame_bcma = BATOM_VEC_INITIALIZER((void *)b_media_frame_bcma, sizeof(b_media_frame_bcma));

static const uint8_t b_media_frame_bcmv[] = { 'B', 'C', 'M', 'V' };
const batom_vec bmedia_frame_bcmv = BATOM_VEC_INITIALIZER((void *)b_media_frame_bcmv, sizeof(b_media_frame_bcmv));

static const uint8_t b_media_eos_bcmv[] = {'B', 'C', 'M', 'V', 0, 0, 0, 4 /* BCMV */ +4 /*size*/ +2/*type*/ +4/*offset*/, 0, 0xFF, 0, 0, 0, 0};
const batom_vec bmedia_eos_bcmv = BATOM_VEC_INITIALIZER((void *)b_media_eos_bcmv, sizeof(b_media_eos_bcmv));

static const uint8_t b_media_vc1_frame_scode[4]={0x00, 0x00, 0x01, 0x0D};
const batom_vec bmedia_frame_vc1 = BATOM_VEC_INITIALIZER((void *)b_media_vc1_frame_scode, sizeof(b_media_vc1_frame_scode));

static const uint8_t b_media_eos_vc1[] = { 0x00, 0x00, 0x01, 0x0A };
const batom_vec bmedia_eos_vc1 = BATOM_VEC_INITIALIZER((void *)b_media_eos_vc1, sizeof(b_media_eos_vc1));

static const uint8_t b_media_eos_stuffing[2*188] = { 0x00 };
const batom_vec bmedia_eos_stuffing = BATOM_VEC_INITIALIZER((void *)b_media_eos_stuffing, sizeof(b_media_eos_stuffing));

const batom_vec bmedia_null_vec = BATOM_VEC_INITIALIZER(0,0);

static const uint8_t b_media_rai_h264[]={0x00, 0x00, 0x00, 0x01, 0x06, 0x06, 0x01, 0xC4, 0x80};
const batom_vec bmedia_rai_h264 = BATOM_VEC_INITIALIZER((void *)b_media_rai_h264, sizeof(b_media_rai_h264));

static const uint8_t b_media_aud_h264[]={0x00, 0x00 ,0x01, 0x09, 0x00, 0x00, 0x01, 0x09}; /* need to send two Access Unit Delimiter's since one is buffered by the RAVE HW/FW/INTERFACE */
const batom_vec bmedia_aud_h264 = BATOM_VEC_INITIALIZER((void *)b_media_aud_h264, sizeof(b_media_aud_h264));

static const uint8_t b_media_divx311_seq_1[] = { 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x20, 0x08, 0xC8, 0x0D, 0x40, 0x00, 0x53, 0x88, 0x40, 0x0C, 0x40, 0x01, 0x90, 0x00, 0x97, 0x53, 0x0A };
const batom_vec bmedia_divx311_seq_1 = BATOM_VEC_INITIALIZER((void *)b_media_divx311_seq_1, sizeof(b_media_divx311_seq_1));

static const uint8_t b_media_divx311_seq_2[] = { 0x30, 0x7F, 0x00, 0x00, 0x01, 0xB2, 0x44, 0x69, 0x76, 0x58, 0x33, 0x31, 0x31, 0x41, 0x4E, 0x44 };
const batom_vec bmedia_divx311_seq_2 = BATOM_VEC_INITIALIZER((void *)b_media_divx311_seq_2, sizeof(b_media_divx311_seq_2));

static const uint8_t b_media_divx4_seq_data[] = { 0x00, 0x00, 0x01, 0xB2, 0x44, 0x69, 0x76, 0x58, 0x34, 0x41, 0x4e, 0x44 };
const batom_vec bmedia_divx4_seq = BATOM_VEC_INITIALIZER((void *)b_media_divx4_seq_data, sizeof(b_media_divx4_seq_data));

static const uint8_t b_media_nal_vec[] = { 0x00, 0x00, 0x01};
const batom_vec bmedia_nal_vec = BATOM_VEC_INITIALIZER((void *)b_media_nal_vec, sizeof(b_media_nal_vec));

const batom_user bmedia_bcma_atom = {
    NULL,
    sizeof(bmedia_bcma_hdr)
};

const batom_user bmedia_adts_atom = {
    NULL,
    sizeof(bmedia_adts_hdr)
};

const batom_user bmedia_bcmv_atom = {
    NULL,
    sizeof(bmedia_bcmv_hdr)
};

void
bmedia_bcma_hdr_init(bmedia_bcma_hdr *hdr, size_t payload_len)
{
    BDBG_CASSERT(sizeof(hdr->pkt_len)>=sizeof(uint32_t));
    hdr->pes.header_len = sizeof(uint32_t);
    hdr->pkt_len[0] = B_GET_BITS(payload_len,31,24);
    hdr->pkt_len[1] = B_GET_BITS(payload_len,23,16);
    hdr->pkt_len[2] = B_GET_BITS(payload_len,15,8);
    hdr->pkt_len[3] = B_GET_BITS(payload_len,7,0);
    return;
}

void bmedia_bcmv_hdr_init(bmedia_bcmv_hdr *hdr, size_t payload_length)
{
    hdr->pes.header_off = 4;
    hdr->pes.header_type = 0;
    hdr->pes.header_len = sizeof(hdr->header);
    payload_length += bmedia_frame_bcmv.len+sizeof(hdr->header);
    hdr->header[0] = B_GET_BITS(payload_length,31,24);
    hdr->header[1] = B_GET_BITS(payload_length,23,16);
    hdr->header[2] = B_GET_BITS(payload_length,15,8);
    hdr->header[3] = B_GET_BITS(payload_length,7,0);
    hdr->header[4] = 0;
    hdr->header[5] = 0; /* type */
    return;
}

void
bmedia_adts_hdr_init(bmedia_adts_hdr *hdr, const bmedia_adts_header *adts, size_t payload_len)
{
    hdr->pes.key_frame = true;
    hdr->pes.header_off = 0;
    hdr->pes.header_len = BMEDIA_ADTS_HEADER_SIZE;
    bmedia_adts_header_fill(hdr->adts, adts, payload_len);
}

bool
bmedia_read_guid(batom_cursor *c, bmedia_guid *guid)
{
	static const uint8_t order[]={3,2,1,0, 5,4, 7,6,  8,9, 10,11,12,13,14,15};
	unsigned i;

#if B_ATOM_FAST	
	if (c->left>=16) {
		for(i=0;i<sizeof(order);i++) {
			guid->guid[order[i]] = c->cursor[i];
		}
		c->cursor+=16;
		c->left-=16;
		return true;
	}
#endif
	for(i=0;i<sizeof(order);i++) {
		guid->guid[order[i]]=batom_cursor_byte(c);
	}
    return BATOM_IS_EOF(c);
}

int 
bmedia_compare_guid(const bmedia_guid *g1, const bmedia_guid *g2)
{
	unsigned i;
	int diff;

	for(i=0;i<sizeof(g1->guid);i++) {
		diff = (int)g1->guid[i]-(int)g2->guid[i];
		if (diff!=0) {
			break;
		}
	}
	return diff;
}

const char*
bmedia_guid2str(const bmedia_guid *guid, char *str, size_t size)
{
	BKNI_Snprintf(str, size,
			"%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X", 
			guid->guid[0], guid->guid[1], guid->guid[2],  guid->guid[3],  guid->guid[4],  guid->guid[5],  guid->guid[6],  guid->guid[7],
			guid->guid[8], guid->guid[9], guid->guid[10], guid->guid[11], guid->guid[12], guid->guid[13], guid->guid[14], guid->guid[15]);
	return str;
}

bool
bmedia_read_waveformatextensible(const bmedia_waveformatex *wf, bmedia_waveformatextensible *wfe)
{
    batom_vec vec;
    batom_cursor cursor;
    const size_t wave_format_len = B_MEDIA_WAV_HEADER_SIZE+BMEDIA_FIELD_LEN(cbSize, uint16_t);

    if(wf->wFormatTag!=0xFFFE) {
        return false;
    }
    if(wf->meta_length<wave_format_len) {
        return false;
    }
    BATOM_VEC_INIT(&vec, wf->meta+wave_format_len, wf->meta_length-wave_format_len);
    batom_cursor_from_vec(&cursor, &vec, 1);
    wfe->samples.validBitsPerSample = batom_cursor_uint16_le(&cursor);
    wfe->channelMask = batom_cursor_uint32_le(&cursor);
    bmedia_read_guid(&cursor, &wfe->subformat);
#if BDBG_DEBUG_BUILD
    if(!BATOM_IS_EOF(&cursor)) {
        char guid[64];
        bmedia_guid2str(&wfe->subformat, guid, sizeof(guid));
        BDBG_MSG(("waveformatextensible: validBitsPerSample:%u channelMask:%#x guid:%s", wfe->samples.validBitsPerSample, wfe->channelMask, guid));
    }
#endif
    return !BATOM_IS_EOF(&cursor);
}

bool
bmedia_waveformatex_is_pcm(const bmedia_waveformatex *wf)
{
    bmedia_waveformatextensible wfe;
    if(wf->wFormatTag==BMEDIA_WAVFMTEX_AUDIO_PCM_TAG) {
        return true;
    } else if(bmedia_read_waveformatextensible(wf, &wfe)) {
        static const bmedia_guid subtypePcm = {{0x00,0x00,0x00,0x01,/*-*/0x00,0x00,/*-*/0x00,0x10,/*-*/0x80,0x00,/*-*/0x00,0xaa,0x00,0x38,0x9b,0x71}};
        if(bmedia_compare_guid(&wfe.subformat, &subtypePcm)==0) {
            return true;
        }
    }
    return false;
}

bool
bmedia_waveformatex_is_ac3(const bmedia_waveformatex *wf)
{
    bmedia_waveformatextensible wfe;
    if(wf->wFormatTag==0x2000) {
        return true;
    } else if(bmedia_read_waveformatextensible(wf, &wfe)) {
        static const bmedia_guid subtypePcm = {{0x00,0x00,0x20,0x00,/*-*/0x00,0x00,/*-*/0x00,0x10,/*-*/0x80,0x00,/*-*/0x00,0xaa,0x00,0x38,0x9b,0x71}}; /* 0x2000 is the key */
        if(bmedia_compare_guid(&wfe.subformat, &subtypePcm)==0) {
            return true;
        }
    }
    return false;
}

batom_t
bmedia_pes_subdivide_packet(batom_accum_t acc_src, bmedia_pes_info *pes_info, batom_accum_t acc_dst, uint8_t *hdr_buf, size_t hdr_buf_size, size_t max_pes_size)
{    
    batom_cursor cursor;
    batom_t result;
    unsigned count=0;
          
    batom_accum_clear(acc_dst); 
    batom_cursor_from_accum(&cursor, acc_src);
   
    for(;;) {
        size_t hdr_len;
        size_t es_len;
        batom_cursor start;
        
		BATOM_CLONE(&start, &cursor);
        if(hdr_buf_size<BMEDIA_PES_HEADER_MAX_SIZE) {
            BDBG_ERR(("bmedia_pes_subdivide_packet: not enough memory for the PES header"));
            break;
        }
        
        es_len = batom_cursor_skip(&cursor, max_pes_size);
        if(es_len==0) {            
            break;
        }
        hdr_len = bmedia_pes_header_init(hdr_buf, es_len, pes_info);
        batom_accum_add_range(acc_dst, hdr_buf, hdr_len);
        batom_accum_append(acc_dst, acc_src, &start, &cursor);
        BMEDIA_PES_UNSET_PTS(pes_info);
        hdr_buf += hdr_len;
        BDBG_ASSERT(hdr_buf_size>=hdr_len);
        hdr_buf_size -= hdr_len;
        count++;
        if(es_len!=max_pes_size) {
            break;
        }
    }
    BDBG_MSG(("bmedia_pes_subdivide_packet: divided %#x:%u frame to %u:%u packets", pes_info->stream_id, (unsigned)batom_accum_len(acc_src), count, (unsigned)max_pes_size));
    batom_accum_clear(acc_src);
    result = batom_from_accum(acc_dst, NULL, NULL);
    return result;
}

bool
bmedia_is_dts_cd(batom_cursor *cursor)
{
    int found_14_bit_sync_word = 0;
    int found_16_bit_sync_word = 0;
    b_media_dts_cd_search_state dts_search_state = b_media_dts_cd_search_state_init;

    while ((found_14_bit_sync_word < 2) && (found_16_bit_sync_word < 2)) {
        uint16_t temp = batom_cursor_uint16_le(cursor);
        if(BATOM_IS_EOF(cursor)) {
            break;
        }
        switch (dts_search_state) {
        case b_media_dts_cd_search_state_init:
            if (temp == 0x1fff) {
                dts_search_state = b_media_dts_cd_search_state_14bit_syncword_1;
            }
            else if (temp == 0x7ffe) {
                dts_search_state = b_media_dts_cd_search_state_16bit_syncword_1;
            }
            else {
                dts_search_state = b_media_dts_cd_search_state_init;
            }
            break;
        case b_media_dts_cd_search_state_16bit_syncword_1:
            if (temp == 0x8001) {
                dts_search_state = b_media_dts_cd_search_state_16bit_syncword_2;
            }
            else if (temp == 0x1fff) {
                dts_search_state = b_media_dts_cd_search_state_14bit_syncword_1;
            }
            else if (temp == 0x7ffe) {
                dts_search_state = b_media_dts_cd_search_state_16bit_syncword_1;
            }
            else {
                dts_search_state = b_media_dts_cd_search_state_init;
            }
            break;
        case b_media_dts_cd_search_state_14bit_syncword_1:
            if (temp == 0xe800) {
                dts_search_state = b_media_dts_cd_search_state_14bit_syncword_2;
            }
            else if (temp == 0x7ffe) {
                dts_search_state = b_media_dts_cd_search_state_16bit_syncword_1;
            }
            else if (temp == 0x1fff) {
                dts_search_state = b_media_dts_cd_search_state_14bit_syncword_1;
            }
            else {
                dts_search_state = b_media_dts_cd_search_state_init;
            }
            break;
        case b_media_dts_cd_search_state_16bit_syncword_2:
            if ((temp & 0xfc00) == 0xfc00) {
                found_16_bit_sync_word++;
                dts_search_state = b_media_dts_cd_search_state_init;
            }
            else if (temp == 0x1fff) {
                dts_search_state = b_media_dts_cd_search_state_14bit_syncword_1;
            }
            else if (temp == 0x7ffe) {
                dts_search_state = b_media_dts_cd_search_state_16bit_syncword_1;
            }
            else {
                dts_search_state = b_media_dts_cd_search_state_init;
            }
            break;
        case b_media_dts_cd_search_state_14bit_syncword_2:
            if ((temp & 0xfff0) == 0x07f0) {
                found_14_bit_sync_word++;
                dts_search_state = b_media_dts_cd_search_state_init;
            }
            else if (temp == 0x1fff) {
                dts_search_state = b_media_dts_cd_search_state_14bit_syncword_1;
            }
            else if (temp == 0x7ffe) {
                dts_search_state = b_media_dts_cd_search_state_16bit_syncword_1;
            }
            else {
                dts_search_state = b_media_dts_cd_search_state_init;
            }
            break;
        }
    }
    return ((found_14_bit_sync_word > 1) || (found_16_bit_sync_word > 1));
}

unsigned
bmedia_waveformatex_pcm_block_size(const bmedia_waveformatex *wf)
{
    unsigned pcm_packet_size = 0;
    if(BMEDIA_WAVFMTEX_AUDIO_PCM(wf) || wf->wFormatTag == BMEDIA_WAVFMTEX_AUDIO_PCM_BE_TAG ) {
        pcm_packet_size = (wf->nSamplesPerSec * 30) / 1000; /* calculate packet size, so it contains 30msec worth of PCM samples */
        pcm_packet_size *= (wf->nChannels * wf->wBitsPerSample)/8; /* make sure that packet size remains aligned on the channel+sample boundary */
    } else if(BMEDIA_WAVFMTEX_AUDIO_MS_ADPCM(wf) || BMEDIA_WAVFMTEX_AUDIO_DVI_ADPCM(wf) || BMEDIA_WAVFMTEX_AUDIO_G711(wf) || wf->wFormatTag==BMEDIA_WAVFMTEX_AUDIO_BCMA_QT_IMA4_TAG  ) {
        pcm_packet_size = (1024 * wf->nSamplesPerSec)/44100;/* calculate packet size, so it contains 23msec worth of samples */
        if(pcm_packet_size<256) { /* but not less than 256 samples */
            pcm_packet_size=256;
        }
        pcm_packet_size *= wf->nChannels;
        if(wf->nBlockAlign>0) {
            pcm_packet_size += wf->nBlockAlign - 1; /* align packet size with nBlockAlign */
            pcm_packet_size -= (pcm_packet_size%wf->nBlockAlign);
        }
    } else {
        pcm_packet_size = (wf->nAvgBytesPerSec/30); /* calculate packet size, so it contains 33msec worth of samples */
        if(wf->nBlockAlign) {
            pcm_packet_size -= pcm_packet_size%wf->nBlockAlign;
        }
    }
    return pcm_packet_size;
}

static bool
bmedia_read_h264_meta_data(bmedia_h264_meta_data *meta, batom_cursor *cursor, unsigned sets)
{
    unsigned i;

    meta->no = sets;
    meta->data = cursor->cursor;
    for(i=0;i<sets;i++) {
        unsigned size = batom_cursor_uint16_be(cursor);

        if(BATOM_IS_EOF(cursor)) {
            return false;
        }
        if(size!=batom_cursor_skip(cursor, size)) {
            return false;
        }
    }
    return true;
}

bool
bmedia_read_h264_meta(bmedia_h264_meta *meta, const void *data, size_t data_len)
{
    batom_vec vec;
    batom_cursor cursor;
    int word;
    unsigned sets;

    BDBG_ASSERT(data);
    BDBG_ASSERT(meta);

    batom_vec_init(&vec, data,  data_len);
    batom_cursor_from_vec(&cursor, &vec, 1);

    /* V_MPEG4/ISO/AVC ID http://haali.cs.msu.ru/mkv/codecs.pdf */
    meta->configurationVersion = batom_cursor_byte(&cursor);
    meta->profileIndication= batom_cursor_byte(&cursor);
    meta->profileCompatibility = batom_cursor_byte(&cursor);
    meta->levelIndication = batom_cursor_byte(&cursor);
    word = batom_cursor_next(&cursor);
    meta->nalu_len = 1+B_GET_BITS(word, 1, 0);
    word = batom_cursor_next(&cursor);
    if(word==BATOM_EOF) {
        return false;
    }
    sets = B_GET_BITS(word,4,0);
    if(!bmedia_read_h264_meta_data(&meta->sps, &cursor, sets)) {
        return false;
    }
    word = batom_cursor_next(&cursor);
    if(word==BATOM_EOF) {
        return false;
    }
    sets = (unsigned)word;
    if(!bmedia_read_h264_meta_data(&meta->pps, &cursor, sets)) {
        return false;
    }
    BDBG_MSG(("bmedia_read_h264_meta: nalu_len:%u sps:%u pps:%u", (unsigned)meta->nalu_len, (unsigned)meta->sps.no, (unsigned)meta->pps.no));
    return true;
}

const void *
bmedia_seek_h264_meta_data(bmedia_h264_meta_data *meta_data, unsigned n, size_t *size)
{
    unsigned i;
    const uint8_t *data;
    BDBG_ASSERT(meta_data);
    BDBG_ASSERT(size);

    data = meta_data->data;
    for(i=0;i<meta_data->no;i++) {
        size_t len = (((uint16_t)data[0]) <<8) | data[1];
        data += 2;
        if(i==n) {
            *size = len;
            return data;
        }
        data += len;
    }
    *size = 0;
    return NULL;
}

static void
bmedia_copy_h264_meta_data(batom_accum_t dst, const bmedia_h264_meta_data *meta_data, const batom_vec *nal_vec)
{
    unsigned i;
    const uint8_t *data;
    size_t entries;

    BDBG_ASSERT(dst);
    BDBG_ASSERT(meta_data);
    data = meta_data->data;
    entries = meta_data->no;

    for(i=0;i<entries;i++) {
        size_t len = (((uint16_t)data[0]) <<8) | data[1];
        data += 2;
        batom_accum_add_vec(dst, nal_vec);
        batom_accum_add_range(dst, data, len);
        data += len;
    }
    return;
}

void
bmedia_copy_h264_meta_with_nal_vec(batom_accum_t dst, const bmedia_h264_meta *meta, const batom_vec *nal_vec)
{
    BDBG_ASSERT(dst);
    BDBG_ASSERT(meta);

    bmedia_copy_h264_meta_data(dst, &meta->sps, nal_vec);
    bmedia_copy_h264_meta_data(dst, &meta->pps, nal_vec);
    return;
}

void
bmedia_copy_h264_meta(batom_accum_t dst, const bmedia_h264_meta *meta)
{
    bmedia_copy_h264_meta_with_nal_vec(dst, meta, &bmedia_nal_vec);
    batom_accum_add_vec(dst, &bmedia_rai_h264);
    return;
}

struct bmedia_p_h265_array_entry {
    uint8_t NAL_unit_type;
    uint16_t numNalus;
};

static bool bmedia_p_h265_array_entry_load(batom_cursor *cursor, struct bmedia_p_h265_array_entry *entry)
{
    unsigned data;

    data = batom_cursor_byte(cursor);
    entry->NAL_unit_type = B_GET_BITS(data,5,0);
    entry->numNalus = batom_cursor_uint16_be(cursor);
    return !BATOM_IS_EOF(cursor);
}

struct bmedia_p_h265_array_estimate {
    unsigned numNalus;
    size_t payload;
};

static bool bmedia_p_estimate_h265_data(const batom_cursor *srcCursor, unsigned numOfArrays, struct bmedia_p_h265_array_estimate *estimate)
{
    unsigned i;
    batom_cursor cursor;
    estimate->payload = 0;
    estimate->numNalus = 0;

    BATOM_CLONE(&cursor, srcCursor);
    for(i=0;i<numOfArrays;i++) {
        struct bmedia_p_h265_array_entry entry;
        unsigned j;

        if(!bmedia_p_h265_array_entry_load(&cursor, &entry)) {
            return false;
        }
        estimate->numNalus += entry.numNalus;
        for(j=0;j<entry.numNalus;j++) {
            unsigned nalUnitLength = batom_cursor_uint16_be(&cursor);
            if(BATOM_IS_EOF(&cursor)) {
                return false;
            }
            estimate->payload += nalUnitLength;
            batom_cursor_skip(&cursor, nalUnitLength);
        }
    }
    if(BATOM_IS_EOF(&cursor)) {
        return false;
    }

    BDBG_MSG(("bmedia_p_estimate_h265_data: numNalus:%u payload:%u", estimate->numNalus, (unsigned)estimate->payload));
    return true;
}

static bool
bmedia_p_load_h265_data(batom_cursor *cursor, unsigned numOfArrays, bmedia_h265_meta *meta)
{
    unsigned i,nal;
    uint8_t *payload;

    payload = (void *)(meta->data + meta->numNalus);
    for(nal=0,i=0;i<numOfArrays;i++) {
        struct bmedia_p_h265_array_entry entry;
        unsigned j;

        if(!bmedia_p_h265_array_entry_load(cursor, &entry)) {
            return false;
        }
        for(j=0;j<entry.numNalus;j++) {
            unsigned nalUnitLength = batom_cursor_uint16_be(cursor);

            if(nal>=meta->numNalus) {
                return false;
            }
            if(BATOM_IS_EOF(cursor)) {
                return false;
            }
            BATOM_VEC_INIT(&meta->data[nal].vec, payload, nalUnitLength);
            batom_cursor_copy(cursor, payload, nalUnitLength);
            payload += nalUnitLength;
            nal++;
        }
    }
    if(nal!=meta->numNalus) {
        return false;
    }
    return true;
}

bool
bmedia_read_h265_meta(batom_cursor *cursor, bmedia_h265_meta *meta, balloc_iface_t alloc)
{
    uint32_t data;
    unsigned numOfArrays;
    size_t metaSize;
    struct bmedia_p_h265_array_estimate estimate;

    /*
      ISO/IEC 14496-15 Third edition
       * 8.3.3.1.2    Syntax
    */

    meta->data = NULL;
    data = batom_cursor_byte(cursor);
    meta->general_profile_space = B_GET_BITS(data,7,6);
    meta->general_tier_flag = B_GET_BIT(data,5);
    meta->general_profile_idc = B_GET_BITS(data,4,0);
    meta->general_profile_compatibility_flags = batom_cursor_uint32_be(cursor);
    meta->general_constraint_indicator_flags = batom_cursor_uint16_be(cursor);
    data = batom_cursor_uint32_be(cursor);
    meta->general_constraint_indicator_flags = (meta->general_constraint_indicator_flags << 32) | data;
    meta->general_level_idc = batom_cursor_byte(cursor);
    data = batom_cursor_uint16_be(cursor);
    meta->min_spatial_segmentation_idc = B_GET_BITS(data,11,0);
    data = batom_cursor_byte(cursor);
    meta->parallelismType = B_GET_BITS(data,1,0);
    data = batom_cursor_byte(cursor);
    meta->chromaFormat = B_GET_BITS(data,1,0);
    data = batom_cursor_byte(cursor);
    meta->bitDepthLuma = 8 + B_GET_BITS(data,2,0);
    data = batom_cursor_byte(cursor);
    meta->bitDepthChroma = 8 + B_GET_BITS(data,2,0);
    meta->avgFrameRate = batom_cursor_uint16_be(cursor);
    data = batom_cursor_byte(cursor);
    meta->constantFrameRate = B_GET_BITS(data,7,6);
    meta->numTemporalLayers = B_GET_BITS(data,5,3);
    meta->temporalIdNested = B_GET_BIT(data,2);
    meta->lengthSize = 1+B_GET_BITS(data,1,0);
    numOfArrays = batom_cursor_byte(cursor);
    if (BATOM_IS_EOF(cursor)) {
        return false;
    }

    BDBG_MSG(("bmedia_h265_meta_read: lengthSize:%u numOfArrays:%u", (unsigned)meta->lengthSize, numOfArrays));
    if(numOfArrays==0) {
        /* no metadata */
        meta->data = NULL;
        meta->numNalus = 0;
        return true;
    }

    if(!bmedia_p_estimate_h265_data(cursor, numOfArrays, &estimate)) {
        return false;
    }
    metaSize = estimate.numNalus * sizeof(meta->data[0]) + estimate.payload;
    meta->numNalus = estimate.numNalus;
    meta->data = alloc->bmem_alloc(alloc, metaSize);
    if(meta->data==NULL) {
        return false;
    }

    if(!bmedia_p_load_h265_data(cursor, numOfArrays, meta)) {
        alloc->bmem_free(alloc, meta->data);
        return false;
    }

    BDBG_MSG(("bmedia_h265_meta_read: metaSize:%u bitDepth:%u,%u avgFrameRate:%u", (unsigned)metaSize, meta->bitDepthLuma, meta->bitDepthChroma, meta->avgFrameRate/256));
    return true;
}

void
bmedia_shutdown_h265_meta(bmedia_h265_meta *meta, balloc_iface_t alloc)
{
    if(meta->data) {
        alloc->bmem_free(alloc, meta->data);
        meta->data = NULL;
    }
    return;
}

void
bmedia_copy_h265_meta(batom_accum_t dst, const bmedia_h265_meta *meta, const batom_vec *nal_vec)
{
    unsigned i;

    for(i=0;i<meta->numNalus;i++) {
        batom_accum_add_vec(dst, nal_vec);
        batom_accum_add_vec(dst, &meta->data[i].vec);
    }
    return;
}

void bmedia_build_div3_seq_hdr_insert_bi(const bmedia_bitmapinfo *bi, void *buf)
{
    uint8_t *hdr=buf;
    hdr[0] = B_GET_BITS(bi->biWidth,11,4);
    hdr[1] =
        B_SET_BITS("width [3..0]", B_GET_BITS(bi->biWidth ,3,0), 7, 4) |
        B_SET_BITS("'10'", 0x02, 3, 2) |
        B_SET_BITS("height [11..10]", B_GET_BITS(bi->biHeight,11,10), 1, 0);
    hdr[2] = B_GET_BITS(bi->biHeight,9,2);
    hdr[3] =
        B_SET_BITS("height [1.0]", B_GET_BITS(bi->biHeight,1,0), 7, 6) |
        B_SET_BITS("'100000'", 0x20, 5, 0);
    return;
}

void bmedia_build_div3_seq_hdr(const bmedia_bitmapinfo *bi, void *buf, size_t buf_size)
{
    uint8_t *hdr=buf;
    BDBG_ASSERT(buf_size >= BMEDIA_DIVX_SEQ_HDR_LEN);
    BKNI_Memcpy(hdr, bmedia_divx311_seq_1.base, bmedia_divx311_seq_1.len);
    hdr+= bmedia_divx311_seq_1.len;
    bmedia_build_div3_seq_hdr_insert_bi(bi, hdr);
    BKNI_Memcpy(hdr+4, bmedia_divx311_seq_2.base, bmedia_divx311_seq_2.len);
    return ;
}

size_t bmedia_strip_emulation_prevention(batom_cursor *cursor, void *buf, size_t len)
{
    uint32_t accum;
    unsigned offset;
    uint8_t *dst=buf;

    for(offset=0,accum=0xFFFFFFFF;;) {
        int byte;
        if(offset>=len) {
            break;  /* overflow */
        }
        byte = batom_cursor_next(cursor);
        if(byte==BATOM_EOF) {
            break;
        }
        accum = byte | (accum << 8);
        accum = B_GET_BITS(accum, 23, 0);
        if(accum != 0x03) {
            dst[offset] = (uint8_t)byte;
            offset++;
        }
    }
    return offset;
}

size_t bmedia_make_bpp_pkt(uint16_t stream_id, const bmedia_bpp_pkt *bpp, void *pkt_, size_t length)
{
    unsigned i;
    uint8_t *pkt  = pkt_;
    if(length < B_MPEG2PES_BPP_LENGTH) {
        (void)BERR_TRACE(BERR_INVALID_PARAMETER);
        return 0;
    }
    BKNI_Memset(pkt, 0, B_MPEG2PES_BPP_LENGTH);
    pkt[0] = 0x00;
    pkt[1] = 0x00;
    pkt[2] = 0x01;
    pkt[3] = (uint8_t)stream_id;
    pkt[4] = 0x00;
    pkt[5] = 0xB2;
    pkt[6] = 0x81;
    pkt[7] = 0x01;
    pkt[8] = 0x14;
    pkt[9] = 0x80;
    pkt[10] = 0x42;
    pkt[11] = 0x52;
    pkt[12] = 0x43;
    pkt[13] = 0x4D;
    pkt[26] = pkt[27] = pkt[28] = pkt[29] = 0xFF;
    for(i=0;i<sizeof(bpp->data)/sizeof(bpp->data[0]);i++) {
        pkt[30+4*i+0] = B_GET_BITS(bpp->data[i], 31, 24);
        pkt[30+4*i+1] = B_GET_BITS(bpp->data[i], 23, 16);
        pkt[30+4*i+2] = B_GET_BITS(bpp->data[i], 15, 8);
        pkt[30+4*i+3] = B_GET_BITS(bpp->data[i], 7, 0);
    }
    BDBG_CASSERT(B_MPEG2PES_BPP_LENGTH>30+sizeof(bpp->data));
    return B_MPEG2PES_BPP_LENGTH;
}
