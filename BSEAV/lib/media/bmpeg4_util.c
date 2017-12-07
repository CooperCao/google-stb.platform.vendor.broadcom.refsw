/***************************************************************************
 * Copyright (C) 2007-2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * MPEG4 stream parsing 
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bmpeg4_util.h"
#include "biobits.h"

BDBG_MODULE(bmpeg4_util);

#define BDBG_MSG_TRACE(x)	/* BDBG_MSG(x) */

#define BMPEG4_ES_DescrTag	0x03
#define BMPEG4_DecoderConfigDescrTag	0x04
#define BMPEG4_DecSpecificInfoTag	0x05

static unsigned b_mpeg4_GetAudioSpecificConfigLen(const void *audioSpecificConfig, unsigned audioSpecificConfig_len);

static size_t
b_mp4_get_expandable_size(batom_cursor *cursor)
{
	/* ISO/IEC 14496-1 MPEG4 Part-1, page 97 */
	int byte;
	uint8_t nextByte;
	size_t sizeOfInstance;

	for(sizeOfInstance=0;;) {
		byte = batom_cursor_next(cursor);
		if (byte==BATOM_EOF) {
			return 0;
		}
		sizeOfInstance =  (sizeOfInstance<<7) | B_GET_BITS(byte, 6, 0);
		nextByte = B_GET_BIT(byte,7);
		if(!nextByte) {
			break;
		}
	}
	return sizeOfInstance;
}

static size_t
b_mp4_mpeg4_read_class(batom_cursor *cursor, batom_cursor *class, uint8_t tag)
{
	size_t size;
	uint8_t class_tag = batom_cursor_next(cursor);

	BDBG_MSG_TRACE(("b_mp4_mpeg4_read_class: tag %#x expected %#x", class_tag, tag));
	if(class_tag!=tag) {
		return 0;
	}
	BATOM_CLONE(class,cursor);
	size = b_mp4_get_expandable_size(class);
	if(size==0) {
		return 0;
	}
	if (batom_cursor_skip(cursor, size) != size) {
		return 0;
	}
	return size;
}


bool
bmpeg4_parse_es_descriptor(batom_cursor *cursor, bmpeg4_es_descriptor *descriptor)
{
	batom_cursor es_cursor;
	batom_cursor config_cursor;
	batom_cursor dec_cursor;
	int byte;
	size_t size;
    bmedia_info_aac info_aac;

	/* 7.2.6.5 ES_Descriptor, ISO/IEC 14496-1 MPEG4 Part-1, page 33 */
	if(!b_mp4_mpeg4_read_class(cursor, &es_cursor, BMPEG4_ES_DescrTag)) {
		return false;
	}
	descriptor->ES_ID = batom_cursor_uint16_be(&es_cursor);
	byte = batom_cursor_next(&es_cursor);
	if(byte==BATOM_EOF) {
		return false;
	}
	if(B_GET_BIT(byte, 7)) { /* streamDependenceFlag) */
		batom_cursor_skip(&es_cursor, sizeof(uint16_t) /* dependsOn_ES_ID */ );
	}
	if(B_GET_BIT(byte, 6)) { /* URL_Flag */
		int URLlength;
		URLlength = batom_cursor_next(&es_cursor);
		if(URLlength==BATOM_EOF) {
			return false;
		}
		batom_cursor_skip(&es_cursor, URLlength /* URLstring */);
	}
	if(B_GET_BIT(byte, 5)) { /* OCRstreamFlag) */ 
		batom_cursor_skip(&es_cursor, sizeof(uint16_t) /* OCR_ES_Id */ );
	}
	/* 7.2.6.6 DecoderConfigDescriptor, ISO/IEC 14496-1 MPEG4 Part-1, page 35*/
	if(!b_mp4_mpeg4_read_class(&es_cursor, &config_cursor, BMPEG4_DecoderConfigDescrTag)) {
		return false;
	}
	descriptor->objectTypeIndication = batom_cursor_byte(&config_cursor);
	if(descriptor->objectTypeIndication==0x00) {
		return false;
	}
	byte = batom_cursor_next(&config_cursor);
	if(byte==BATOM_EOF) {
		return false;
	}
	descriptor->streamType = B_GET_BITS(byte,7,2);
	batom_cursor_skip(&config_cursor, 3 /* bufferSizeDB */ + sizeof(uint32_t) /* maxBitrate */ + sizeof(uint32_t) /* avgBitrate */);
	switch(descriptor->objectTypeIndication) {
	case BMPEG4_Audio_ISO_IEC_14496_3:
	case BMPEG4_Audio_ISO_IEC_13818_7:
        {
            batom_cursor aac_data;

            size = b_mp4_mpeg4_read_class(&config_cursor, &dec_cursor, BMPEG4_DecSpecificInfoTag);
            if(size==0) {
                return false;
            }
            BATOM_CLONE(&aac_data,&dec_cursor);
            if(!bmedia_info_probe_aac_info(&dec_cursor, &info_aac)) {
                return false;
            }

            descriptor->decoder.iso_14496_3.audioObjectType = info_aac.profile;
            descriptor->decoder.iso_14496_3.channelConfiguration = info_aac.channel_configuration;
            descriptor->decoder.iso_14496_3.samplingFrequencyIndex = info_aac.sampling_frequency_index;
            descriptor->decoder.iso_14496_3.samplingFrequency = info_aac.sampling_frequency;
            descriptor->decoder.iso_14496_3.aac_info_size = 0;

            if( size <= sizeof(descriptor->decoder.iso_14496_3.aac_info)) {
                descriptor->decoder.iso_14496_3.aac_info_size = batom_cursor_copy(&aac_data, descriptor->decoder.iso_14496_3.aac_info, size);
                if(descriptor->decoder.iso_14496_3.aac_info_size == size) {
                    descriptor->decoder.iso_14496_3.aac_info_size_bits = b_mpeg4_GetAudioSpecificConfigLen(descriptor->decoder.iso_14496_3.aac_info, size);
                } else {
                    BDBG_ERR(("missing codec data %u/%u", (unsigned)size, (unsigned)descriptor->decoder.iso_14496_3.aac_info_size));
                    descriptor->decoder.iso_14496_3.aac_info_size = 0;
                    descriptor->decoder.iso_14496_3.aac_info_size_bits = 0;
                }
            } else {
                BDBG_ERR(("not enough space for codec data %u/%u", (unsigned)size, (unsigned)sizeof(descriptor->decoder.iso_14496_3.aac_info)));
            }
            break;
        }
    case BMPEG4_Video_ISO_IEC_14496_2:
        size = b_mp4_mpeg4_read_class(&config_cursor, &dec_cursor, BMPEG4_DecSpecificInfoTag);
        if(size==0) {
            return false;
        }
        if(size>sizeof(descriptor->decoder.iso_14496_2.header)) {
            BDBG_WRN(("bmpeg4_parse_es_descriptor: too large ISO IEC 14496.2 decoder information %u:%u", (unsigned)size, (unsigned)sizeof(descriptor->decoder.iso_14496_2.header)));
            return false;
        }
        descriptor->decoder.iso_14496_2.header_size = batom_cursor_copy(&dec_cursor, descriptor->decoder.iso_14496_2.header, size);
        break;
    }
    return true;
}

/* ISO/IEC 14496-3:2008(E)
 * Table 4.63 – Syntax of sbr_header() */
static bool b_mpeg4_parse_sbr_header(batom_bitstream *bs)
{
    bool bs_amp_res;
    unsigned bs_start_freq;
    unsigned bs_stop_freq;
    unsigned bs_xover_band;
    unsigned bs_reserved;
    bool bs_header_extra_1;
    bool bs_header_extra_2;

    bs_amp_res = batom_bitstream_bit(bs);
    bs_start_freq = batom_bitstream_bits(bs, 4);

    bs_stop_freq = batom_bitstream_bits(bs, 4);

    bs_xover_band = batom_bitstream_bits(bs, 3);

    bs_reserved = batom_bitstream_bits(bs, 2);
    bs_header_extra_1 = batom_bitstream_bit(bs);
    bs_header_extra_2 = batom_bitstream_bit(bs);

    BSTD_UNUSED(bs_amp_res);
    BSTD_UNUSED(bs_start_freq);
    BSTD_UNUSED(bs_stop_freq);
    BSTD_UNUSED(bs_xover_band);
    BSTD_UNUSED(bs_reserved);

    if(bs_header_extra_1) {
        unsigned bs_freq_scale;
        bool bs_alter_scale;
        unsigned bs_noise_bands;

        bs_freq_scale = batom_bitstream_bits(bs, 2);
        bs_alter_scale = batom_bitstream_bit(bs);
        bs_noise_bands = batom_bitstream_bits(bs, 2);

        BSTD_UNUSED(bs_freq_scale);
        BSTD_UNUSED(bs_alter_scale);
        BSTD_UNUSED(bs_noise_bands);
    }
    if(bs_header_extra_2) {
        unsigned bs_limiter_bands;
        unsigned bs_limiter_gains;
        bool bs_interpol_freq;
        bool bs_smoothing_mode;

        bs_limiter_bands = batom_bitstream_bits(bs, 2);
        bs_limiter_gains = batom_bitstream_bits(bs, 2);
        bs_interpol_freq =  batom_bitstream_bit(bs);
        bs_smoothing_mode =  batom_bitstream_bit(bs);

        BSTD_UNUSED(bs_limiter_bands);
        BSTD_UNUSED(bs_limiter_gains);
        BSTD_UNUSED(bs_interpol_freq);
        BSTD_UNUSED(bs_smoothing_mode);
    }

    return !batom_bitstream_eof(bs);
}

/* ISO/IEC 14496-3:2008(E)
   Table 4.181 – Syntax of ld_sbr_header () */
static bool b_mpeg4_parse_ld_sbr_header(batom_bitstream *bs, unsigned channel_configuration)
{
    unsigned numSbrHeader;
    unsigned i;
    switch(channel_configuration) {
    case 1: case 2: numSbrHeader = 1; break;
    case 3: numSbrHeader = 2; break;
    case 4: case 5: case 6: numSbrHeader = 3; break;
    case 7: numSbrHeader = 4; break;
    default: numSbrHeader = 0; break;
    }
    for(i=0;i<numSbrHeader;i++) {
        b_mpeg4_parse_sbr_header(bs);
    }
    return !batom_bitstream_eof(bs);
}

/* ISO/IEC 14496-3:2008(E)
   Table 4.180 – Syntax of ELDSpecificConfig () */
static bool
b_mpeg4_parse_ELDSpecificConfig(batom_bitstream *bs, const bmedia_info_aac *aac)
{
    bool frameLengthFlag;
    bool aacSectionDataResilienceFlag;
    bool aacScalefactorDataResilienceFlag;
    bool aacSpectralDataResilienceFlag;
    bool ldSbrPresentFlag;

    frameLengthFlag = batom_bitstream_bit(bs);
    aacSectionDataResilienceFlag = batom_bitstream_bit(bs);
    aacScalefactorDataResilienceFlag = batom_bitstream_bit(bs);
    aacSpectralDataResilienceFlag = batom_bitstream_bit(bs);

    BSTD_UNUSED(frameLengthFlag);
    BSTD_UNUSED(aacSectionDataResilienceFlag);
    BSTD_UNUSED(aacScalefactorDataResilienceFlag);
    BSTD_UNUSED(aacSpectralDataResilienceFlag);

    ldSbrPresentFlag = batom_bitstream_bit(bs);
    if(ldSbrPresentFlag) {
        bool ldSbrSamplingRate;
        bool ldSbrCrcFlag;

        ldSbrSamplingRate = batom_bitstream_bit(bs);
        ldSbrCrcFlag = batom_bitstream_bit(bs);
        b_mpeg4_parse_ld_sbr_header(bs, aac->channel_configuration);
#if 0
        if(batom_bitstream_eof(bs)) {
            return false;
        }
        aac->sampling_frequency *= 2;
#endif

        BSTD_UNUSED(ldSbrSamplingRate);
        BSTD_UNUSED(ldSbrCrcFlag);
    }
    for(;;) {
        unsigned eldExtType;
        unsigned eldExtLen;

        eldExtType = batom_bitstream_bits(bs, 4);
        if(eldExtType==0) { /* ELDEXT_TERM */
            break;
        }
        eldExtLen = batom_bitstream_bits(bs, 4);
        if(eldExtLen==15) {
            eldExtLen += batom_bitstream_bits(bs, 8);
        }
        if(eldExtLen==255) {
            eldExtLen += batom_bitstream_bits(bs, 16);
        }
        if(batom_bitstream_eof(bs)) {
            break;
        }
        batom_bitstream_drop_bits(bs, eldExtLen);
    }
    return !batom_bitstream_eof(bs);
}

/* ISO/IEC 14496-3:2008(E)
   Table 1.49 – Syntax of ErrorProtectionSpecificConfig() */
static bool
b_mpeg4_parse_ErrorProtectionSpecificConfig(batom_bitstream *bs)
{
    unsigned number_of_predefined_set;
    unsigned interleave_type;
    unsigned bit_stuffing;
    unsigned number_of_concatenated_frame;
    unsigned i;
    bool header_protection;

    number_of_concatenated_frame = batom_bitstream_bits(bs, 8);
    interleave_type = batom_bitstream_bits(bs, 2);
    bit_stuffing = batom_bitstream_bits(bs, 3);
    number_of_concatenated_frame = batom_bitstream_bits(bs, 3);
    if(batom_bitstream_eof(bs)) { return false; }

    for(i=0;i<number_of_concatenated_frame;i++) {
        unsigned number_of_class_i;
        unsigned j;
        bool class_reorder_output;

        number_of_class_i = batom_bitstream_bits(bs, 6);
        if(batom_bitstream_eof(bs)) { return false; }
        for(j=0;j<number_of_class_i;j++) {
            bool length_escape_ij;
            bool rate_escape_ij;
            bool crclen_escape_ij;
            unsigned fec_type_ij;
            bool termination_switch_ij;
            unsigned interleave_switch_ij;
            bool class_optional;

            length_escape_ij = batom_bitstream_bit(bs);
            rate_escape_ij = batom_bitstream_bit(bs);
            crclen_escape_ij = batom_bitstream_bit(bs);
            if(number_of_concatenated_frame != 1) {
                bool concatenate_flags_ij;
                concatenate_flags_ij = batom_bitstream_bit(bs);
                BSTD_UNUSED(concatenate_flags_ij);
            }
            fec_type_ij = batom_bitstream_bits(bs, 2);
            if(fec_type_ij==0) {
                termination_switch_ij = batom_bitstream_bit(bs);
            }
            if(interleave_type==2) {
                interleave_switch_ij = batom_bitstream_bits(bs,2);
            }
            class_optional = batom_bitstream_bit(bs);
            if(length_escape_ij==1) {
                unsigned number_of_bits_for_length_ij;
                number_of_bits_for_length_ij = batom_bitstream_bits(bs,4);
                BSTD_UNUSED(number_of_bits_for_length_ij);
            } else {
                unsigned class_length_ij;
                class_length_ij = batom_bitstream_bits(bs,16);
                BSTD_UNUSED(class_length_ij);
            }
            if(rate_escape_ij != 1) {
                unsigned class_rate;

                if(fec_type_ij) {
                    class_rate = batom_bitstream_bits(bs,7);
                } else {
                    class_rate = batom_bitstream_bits(bs,5);
                }
                BSTD_UNUSED(class_rate);
            }
            if(crclen_escape_ij != 1) {
                unsigned class_crclen_ij;
                class_crclen_ij = batom_bitstream_bits(bs,5);
                BSTD_UNUSED(class_crclen_ij);
            }
            BSTD_UNUSED(class_optional);
            BSTD_UNUSED(interleave_switch_ij);
            BSTD_UNUSED(termination_switch_ij);
        }
        class_reorder_output = batom_bitstream_bit(bs);
        if(class_reorder_output==1) {
            for(j=0;j<number_of_class_i;j++) {
                unsigned class_output_order_ij;
                class_output_order_ij = batom_bitstream_bits(bs,6);
                BSTD_UNUSED(class_output_order_ij);
            }
        }
    }
    header_protection = batom_bitstream_bit(bs);
    if(header_protection) {
        unsigned header_rate;
        unsigned header_crclen;

        header_rate = batom_bitstream_bits(bs, 5);
        header_crclen = batom_bitstream_bits(bs, 5);

        BSTD_UNUSED(header_rate);
        BSTD_UNUSED(header_crclen);
    }
    BSTD_UNUSED(bit_stuffing);
    BSTD_UNUSED(number_of_predefined_set);
    return !batom_bitstream_eof(bs);
}


/* 1.6.2.1 AudoSpecificConfig, ISO/IEC 14496-3 {ed 3.0}, page 36*/
static bool
b_media_info_probe_aac_info( batom_bitstream *bs, bmedia_info_aac *aac)
{
    bool delay_flag;
    bool ext_flag;
    bool fl_flag;
    bool sbr;

    /* Table 1.14 Syntax of GetAudioObjectType() */
    aac->profile = bmpeg4_parse_AudioObjectType(bs);
    aac->sampling_frequency = bmpeg4_parse_SamplingRate(bs, &aac->sampling_frequency_index);

    aac->channel_configuration = batom_bitstream_bits(bs,4);
    if(batom_bitstream_eof(bs)) { goto error_eof; }

    if (aac->profile == 5)
    { /* SBR */
        aac->sampling_frequency = bmpeg4_parse_SamplingRate(bs, &aac->sampling_frequency_index);
        aac->profile =  bmpeg4_parse_AudioObjectType(bs);
        if(batom_bitstream_eof(bs)) { goto error_eof; }
    }
    if (!batom_bitstream_reserve(bs, 8)) {
        goto done;
    }

    switch(aac->profile)
    {  
        case 1:
        case 2:
        case 3:
        case 4:
        case 6:
        case 7:
        case 17:
        case 19:
        case 20:
        case 21:
        case 22:
        case 23:
            /* GASpecificConfig */
            fl_flag = batom_bitstream_bit(bs);
            BSTD_UNUSED(fl_flag);
            delay_flag = batom_bitstream_bit(bs);
            if(batom_bitstream_eof(bs)) { goto error_eof; }
            if(delay_flag) 
            {                    
                /* Delay is 14 bits */
                batom_bitstream_drop_bits(bs, 14);
            }
            
            ext_flag = batom_bitstream_bit(bs);
            if(batom_bitstream_eof(bs)) { goto error_eof; }

            if (aac->profile == 6 ||
                aac->profile == 20)
            {
                batom_bitstream_drop_bits(bs, 13);
            }
            
            if (ext_flag)
            {
                if (aac->profile == 22)
                {
                    batom_bitstream_drop_bits(bs, 16);
                }
                else if (aac->profile == 17 ||
                    aac->profile == 19 ||
                    aac->profile == 20 ||
                    aac->profile == 23)
                {
                    batom_bitstream_drop_bits(bs, 3);
                }
                
                ext_flag = batom_bitstream_bit(bs);
            }
            break;

        case 39:
            b_mpeg4_parse_ELDSpecificConfig(bs, aac);
            break;

        default:                
            break;
    }
    if(batom_bitstream_eof(bs)) { goto error_eof; }
    switch (aac->profile) {
        case 17:
        case 19:
        case 20:
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
        case 26:
        case 27:
        case 39:
            {
                unsigned epConfig;
                epConfig = batom_bitstream_bits(bs,2);
                if(epConfig==2 || epConfig==3) {
                    b_mpeg4_parse_ErrorProtectionSpecificConfig(bs);
                }
                if(epConfig) {
                    bool directMapping;
                    directMapping = batom_bitstream_bit(bs);
                    BSTD_UNUSED(directMapping);
                }
                break;
            }
        default:
            break;
    }
    if(batom_bitstream_eof(bs)) { goto error_eof; }

    if (aac->profile != 5 && batom_bitstream_reserve(bs, 16) )
    {
        unsigned syncExtensionType;
        syncExtensionType = batom_bitstream_bits(bs,11);
        if (syncExtensionType == 0x2b7)
        {
            unsigned extensionAudioObjectType;

            extensionAudioObjectType = bmpeg4_parse_AudioObjectType(bs);
            if(batom_bitstream_eof(bs)) { goto error_eof; }
            if (extensionAudioObjectType == 0x5)
            { 
                sbr = batom_bitstream_bit(bs);
                if(batom_bitstream_eof(bs)) { goto error_eof; }
                if (sbr)
                {
                    aac->profile  = extensionAudioObjectType;
                    aac->sampling_frequency = bmpeg4_parse_SamplingRate(bs, &aac->sampling_frequency_index);
                }
            }
        }
    }

done:
    BDBG_MSG(("aac_info: profile:%u sampling_frequency_index:%u channel_configuration:%u sampling_frequency:%u", aac->profile, aac->sampling_frequency_index, aac->channel_configuration, aac->sampling_frequency));

    return true;

error_eof:
    BDBG_MSG(("bmedia_info_probe_aac_info:%p: Can't parse AudoSpecificConfig", (void *)bs));
    return false;
}

bool
bmedia_info_probe_aac_info(batom_cursor *cursor, bmedia_info_aac *aac)
{
    batom_bitstream bs;

    batom_bitstream_init(&bs, cursor);
    return b_media_info_probe_aac_info(&bs, aac);
}

typedef struct b_bitstream_out {
    void *buf;
    unsigned buf_len;
    unsigned offset;
    uint32_t cache;       /* 32 bits used as a write buffer */
    int cache_pos;       /* number of bits stored in the cache */
} b_bitstream_out;

static void b_bitstream_out_init(b_bitstream_out *bs, void *buf, unsigned buf_len)
{
    BDBG_ASSERT(buf_len%sizeof(bs->cache)==0);
    bs->buf = buf;
    bs->buf_len = buf_len;
    bs->offset = 0;
    bs->cache = 0;
    bs->cache_pos = 0;
    return;
}

static void b_bitstream_out_flush(b_bitstream_out *bs, uint32_t cache)
{
    if(bs->offset + sizeof(cache) < bs->buf_len) {
        B_MEDIA_SAVE_UINT32_BE((uint8_t *)bs->buf + bs->offset, cache);
    }
    return;
}

static void b_bitstream_out_bit(b_bitstream_out *bs, bool bit)
{
    bs->cache = (bs->cache<<1) | (bit?1:0);
    bs->cache_pos++;
    if(bs->cache_pos==32) {
        b_bitstream_out_flush(bs, bs->cache);
        bs->offset = bs->offset + sizeof(bs->cache);
        bs->cache_pos = 0;
        bs->cache = 0;
    }
    return;
}

static void b_bitstream_out_bits(b_bitstream_out *bs, unsigned data, unsigned bits)
{
    unsigned i;
    unsigned mask = 1<<(bits-1);
    for(i=0;i<bits;i++) {
        b_bitstream_out_bit(bs, (data & mask) != 0);
        mask = mask>>1;
    }
    return;
}

static void b_bitstream_out_align(b_bitstream_out *bs, unsigned n)
{
    unsigned extra = bs->cache_pos%n;
    if(extra) {
        b_bitstream_out_bits(bs, 0, n-extra);
    }
    return;
}

static void b_bitstream_out_byte_align(b_bitstream_out *bs)
{
    b_bitstream_out_align(bs, 8);
    return;
}

static void b_bitstream_out_copy_bits(b_bitstream_out *bs, const void *data, unsigned bits)
{
    unsigned i;
    unsigned bytes = bits/8;
    unsigned left;

    for(i=0;i<bytes;i++) {
        uint8_t byte = ((const uint8_t *)data)[i];
        b_bitstream_out_bits(bs, byte, 8);
    }
    left = bits - bytes*8;
    if(left) {
        uint8_t byte = ((const uint8_t *)data)[bytes];
        b_bitstream_out_bits(bs, byte, left);
    }
    return;
}

static void b_bitstream_out_copy_atom(b_bitstream_out *bs, batom_t atom)
{
    batom_cursor cursor;

    batom_cursor_from_atom(&cursor, atom);
    for(;;) {
        int byte;
        BATOM_NEXT(byte, &cursor);
        if(byte == BATOM_EOF) {
            break;
        }
        b_bitstream_out_bits(bs, byte, 8);
    }
    return;
}

static int b_bitstream_out_complete(b_bitstream_out *bs)
{
    unsigned extra;
    if(bs->cache_pos!=0) {
        uint32_t cache = bs->cache;
        cache = cache << (32-bs->cache_pos);
        b_bitstream_out_flush(bs, cache);
    }
    extra = (bs->cache_pos + 7) / 8;
    if(extra + bs->offset >=  bs->buf_len) {
        BDBG_ERR(("b_bitstream_out:%p overflow (%u/%u)", (void *)bs, bs->buf_len, (extra+bs->offset)));
        return -1;
    }
    return extra + bs->offset;
}


/* ISO/IEC 14496-3:2005(E)
 * 1.7 MPEG-4 Audio transport stream
 */

/* Table 1.16 . Sampling Frequency Index */
static const unsigned b_mpeg4_sample_rate[]={
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
	8000,
    7350
};

unsigned
bmpeg4_parse_AudioObjectType(batom_bitstream *bs)
{
    unsigned audioObjectType;
    /* Table 1.14 . Syntax of GetAudioObjectType() */
    audioObjectType = batom_bitstream_bits(bs, 5);
    if(audioObjectType==31) {
        audioObjectType = 32 + batom_bitstream_bits(bs, 6);
    }
    return audioObjectType;
}

unsigned
bmpeg4_parse_SamplingRate(batom_bitstream *bs, uint8_t *index)
{
    unsigned sample_rate=0;
    unsigned samplingFrequencyIndex;

    samplingFrequencyIndex = batom_bitstream_bits(bs, 4);
    if(index) {
        *index = samplingFrequencyIndex;
    }
    if(batom_bitstream_eof(bs)) {
        return 0;
    }
    if(samplingFrequencyIndex==0xf) {
        sample_rate = batom_bitstream_bits(bs, 24);
    } else {
        if(samplingFrequencyIndex<sizeof(b_mpeg4_sample_rate)/sizeof(*b_mpeg4_sample_rate)) {
            sample_rate = b_mpeg4_sample_rate[samplingFrequencyIndex];
        }
    }
    return sample_rate;
}

/* Table 1.13 . Syntax of AudioSpecificConfig() */
static unsigned b_mpeg4_GetAudioSpecificConfigLen(const void *audioSpecificConfig, unsigned audioSpecificConfig_len)
{
    batom_cursor cursor;
    batom_bitstream bs;
    batom_vec vec;
    bmedia_info_aac aac;

    BATOM_VEC_INIT(&vec, audioSpecificConfig, audioSpecificConfig_len);
    batom_cursor_from_vec(&cursor, &vec, 1);
    batom_bitstream_init(&bs, &cursor);

    b_media_info_probe_aac_info(&bs, &aac);
    return batom_bitstream_bit_position(&bs);
}

/* Table 1.29 — Syntax of StreamMuxConfig() */
static void b_StreamMuxConfig(b_bitstream_out *bs, const void *audioSpecificConfig, unsigned audioSpecificConfig_len_bits)
{
    b_bitstream_out_bit(bs, 0 /* audioMuxVersion */);
    b_bitstream_out_bit(bs, 1 /* allStreamsSameTimeFraming */);
    b_bitstream_out_bits(bs, 0 /* numSubFrames*/, 6);
    b_bitstream_out_bits(bs, 0 /* numProgram */, 4);

    b_bitstream_out_bits(bs, 0 /* numLayer */, 3);
    b_bitstream_out_copy_bits(bs, audioSpecificConfig, audioSpecificConfig_len_bits);
    b_bitstream_out_bits(bs, 0 /* frameLengthType */, 3);

    b_bitstream_out_bits(bs, 0xFF /* latmBufferFullness */, 8);

    b_bitstream_out_bit(bs, 0 /* otherDataPresent */);
    b_bitstream_out_bit(bs, 0 /* crcCheckPresent */);

    return;
}

static void b_PayloadLengthInfo(b_bitstream_out *bs,  unsigned frame_len)
{
    for(;;) {
        uint8_t tmp;
        if(frame_len>0xFF) {
            tmp = 0xFF;
        } else {
            tmp = frame_len;
        }
        frame_len -= tmp;
        b_bitstream_out_bits(bs, tmp, 8);
        if(frame_len==0  && tmp!=0xFF) {
            break;
        }
    }
    return;
}


static void b_AudioMuxElement(b_bitstream_out *bs,  unsigned frame_len, const void *audioSpecificConfig, unsigned audioSpecificConfig_len_bits)
{
    /* Table 1.28 — Syntax of AudioMuxElement() */
    b_bitstream_out_bit(bs, 0 /* useSameStreamMux */);
    b_StreamMuxConfig(bs, audioSpecificConfig, audioSpecificConfig_len_bits);
    b_PayloadLengthInfo(bs,  frame_len);
    return;
}


int bmedia_create_loas_packet(void *buf, unsigned buf_len, const void *audioSpecificConfig, unsigned audioSpecificConfig_len_bits, batom_t frame)
{
    b_bitstream_out bs;
    unsigned frame_len;
    int packet_len;


    b_bitstream_out_init(&bs, buf, buf_len);

    /* Table 1.23 — Syntax of AudioSyncStream() */
    b_bitstream_out_bits(&bs, 0x2B7, 11);
    b_bitstream_out_bits(&bs, 0, 13); /* audioMuxLenghtBytes */

    frame_len = batom_len(frame);
    b_AudioMuxElement(&bs,  frame_len, audioSpecificConfig, audioSpecificConfig_len_bits);
    b_bitstream_out_copy_atom(&bs, frame);
    b_bitstream_out_byte_align(&bs);
    packet_len = b_bitstream_out_complete(&bs);
    if(packet_len > 3) {
        unsigned audioMuxLengthBytes = packet_len - 3;
        uint32_t header = B_MEDIA_LOAD_UINT32_BE(buf,0);
        header |= audioMuxLengthBytes << 8;
        B_MEDIA_SAVE_UINT32_BE((uint8_t *)buf, header);
    }
    return packet_len;
}
