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
#ifndef _BMPEG4_UTIL_H__
#define _BMPEG4_UTIL_H__

#include "bioatom.h"
#include "bmedia_util.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* 7.2.6.5 Semantics, Table 5. objectTypeIndication Values ISO/IEC 14496-1 MPEG4 Part-1, page 35 */
#define BMPEG4_Audio_ISO_IEC_14496_3	0x40
#define BMPEG4_Audio_ISO_IEC_13818_7	0x67
#define BMPEG4_Video_ISO_IEC_14496_2	0x20
#define BMPEG4_Audio_ISO_IEC_11172_3    0x6B

typedef struct bmpeg4_es_descriptor {
    uint16_t ES_ID;
    uint8_t objectTypeIndication;
    uint8_t streamType;
    union {
        struct {
            uint8_t audioObjectType;
            uint8_t samplingFrequencyIndex;
            uint8_t channelConfiguration;
            unsigned samplingFrequency; /* only valid if samplingFrequencyIndex==0x0F */
            unsigned aac_info_size; /* in bytes */
            unsigned aac_info_size_bits; /* in bits */
            uint8_t aac_info[3*1024];
        } iso_14496_3;
        struct {
            size_t header_size;
            uint8_t header[128];    /* changed from 64 to 128 to accommodate PR35214: LIB-Coverity (CID 3048): OVERRUN_STATIC */
        } iso_14496_2;
    } decoder;
} bmpeg4_es_descriptor;


bool
bmpeg4_parse_es_descriptor(batom_cursor *cursor, bmpeg4_es_descriptor *descriptor);

bool
bmedia_info_probe_aac_info(batom_cursor *cursor, bmedia_info_aac *aac);


int bmedia_create_loas_packet(void *buf, unsigned buf_len, const void *audioSpecificConfig, unsigned audioSpecificConfig_len_bits, batom_t frame);

unsigned bmpeg4_parse_SamplingRate(batom_bitstream *bs, uint8_t *index);
unsigned bmpeg4_parse_AudioObjectType(batom_bitstream *bs);

#ifdef __cplusplus
}
#endif


#endif /* _BMPEG4_UTIL_H__ */

