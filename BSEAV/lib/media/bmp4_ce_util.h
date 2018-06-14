/***************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
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
 * MP4 container parser library (Common encryption in ISO base media file format)
 *******************************************************************************/
#ifndef _BMP4_CE_UTIL_H__
#define _BMP4_CE_UTIL_H__

#include "bmp4_util.h"

#ifdef __cplusplus
extern "C"
{
#endif


struct bmp4_CencSampleEncryptionInformationGroupEntry {
    uint8_t crypt_byte_block;
    uint8_t skip_byte_block;
    uint8_t isProtected;
    uint8_t Per_Sample_IV_Size;
    uint8_t KID[16];
    uint8_t constant_IV_size;
    uint8_t constant_IV[16];
};

struct bmp4_TrackEncryption {
    uint8_t default_crypt_byte_block;
    uint8_t default_skip_byte_block;
    bool default_crypt_skip_valid;
    uint8_t default_isProtected;
    uint8_t default_Per_Sample_IV_Size;
    uint8_t default_KID[16];
    uint8_t default_constant_IV_size;
    uint8_t default_constant_IV[16];
    bool default_constant_IV_valid;
};

struct bmp4_SampleEncryption {
    uint32_t sample_count;
};

struct bmp4_SampleEncryptionEntry {
    uint8_t InitializationVector[16];
    uint16_t subsample_count;
};

struct bmp4_SampleEncryptionSubsample {
    uint16_t BytesOfClearData;
    uint32_t BytesOfProtectedData;
};

struct bmp4_SchemeType {
    uint32_t scheme_type;
    uint32_t scheme_version;
    bool scheme_uri_valid;
    char scheme_uri[128];
};


#define BMP4_SAMPLE_ENCRYPTION  BMP4_TYPE('s','e','n','c')
#define BMP4_TRACK_ENCRYPTION BMP4_TYPE('t','e','n','c')

#define BMP4_CENC_SAMPLE_ENCRYPTION_INFORMATION_GROUP_ENTRY BMP4_TYPE('s','e','i','g')
#define BMP4_SCHEME_TYPE BMP4_TYPE('s','c','h','m')
#define BMP4_SCHEME_INFORMATION  BMP4_TYPE('s','c','h','i')

#define BMP4_CENC_SCHEME_AES_CTR BMP4_TYPE('c','e','n','c')

bool bmp4_parse_CencSampleEncryptionInformationGroupEntry(batom_cursor *cursor, struct bmp4_CencSampleEncryptionInformationGroupEntry *seig);
bool bmp4_parse_TrackEncryption(batom_cursor *cursor, const bmp4_fullbox *box, struct bmp4_TrackEncryption *tenc);
bool bmp4_parse_SampleEncryption(batom_cursor *cursor, struct bmp4_SampleEncryption *senc);
bool bmp4_parse_SampleEncryptionEntry(batom_cursor *cursor, const bmp4_fullbox *box, unsigned Per_Sample_IV_Size, struct bmp4_SampleEncryptionEntry *entry);
bool bmp4_parse_SampleEncryptionSubsample(batom_cursor *cursor, struct bmp4_SampleEncryptionSubsample *subsample);
bool bmp4_parse_SchemeType(batom_cursor *cursor, const bmp4_fullbox *box, struct bmp4_SchemeType *schm);


struct b_mp4_cenc_subsample {
    unsigned offset;
    unsigned count;
};

#define B_MP4_CENC_MAX_FRAME_VECS   256
#define B_MP4_CENC_MAX_FRAME_SUBSAMPLES 64
struct b_mp4_cenc_frame {
    unsigned vec_count;
    unsigned subsample_count;
    struct b_mp4_cenc_subsample subsamples[B_MP4_CENC_MAX_FRAME_SUBSAMPLES];
    batom_vec vecs[B_MP4_CENC_MAX_FRAME_VECS];
};

unsigned b_mp4_process_payload_cenc_subsample_cens_buffer_len(const struct b_mp4_cenc_frame *frame, unsigned crypt_byte_block, unsigned skip_byte_block);
BERR_Code b_mp4_process_payload_cenc_subsample_cens_buffer_iterate(const struct b_mp4_cenc_frame *frame, unsigned crypt_byte_block, unsigned skip_byte_block, void *context, BERR_Code (*apply)(void *, unsigned offset, void *, unsigned)  );
BERR_Code b_mp4_process_payload_cenc_subsample_accumulate(struct b_mp4_cenc_frame *frame, const batom_vec *vecs, unsigned nvecs);

struct b_mp4_cenc_payload_handler {
    BERR_Code (*sample_begin)(void *context, struct bmp4_SampleEncryptionEntry *senc, const struct bmp4_CencSampleEncryptionInformationGroupEntry *group);
    BERR_Code (*subsample)(void *context, struct b_mp4_cenc_frame *frame, const batom_vec *vecs, unsigned nvecs);
    BERR_Code (*sample_end)(void *context, struct b_mp4_cenc_frame *frame, const struct bmp4_CencSampleEncryptionInformationGroupEntry *group);
};

#ifdef __cplusplus
}
#endif


#endif /* _BMP4_CE_UTIL_H__ */
