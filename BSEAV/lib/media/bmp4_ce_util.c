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
#include "bstd.h"
#include "bmp4_util.h"
#include "bkni.h"
#include "biobits.h"
#include "bmp4_ce_util.h"

BDBG_MODULE(bmp4_ce_util);

/* ISO/IEC 23001-7 3rd Edition  6. Encryption parameters shared by groups of samples */
bool bmp4_parse_CencSampleEncryptionInformationGroupEntry(batom_cursor *cursor, struct bmp4_CencSampleEncryptionInformationGroupEntry *seig)
{
    int temp;
    unsigned i;

    batom_cursor_skip(cursor, 1); /* reserved */
    temp = batom_cursor_next(cursor);
    seig->crypt_byte_block = B_GET_BITS(temp, 8, 4);
    seig->skip_byte_block = B_GET_BITS(temp, 4, 0);
    seig->isProtected = batom_cursor_byte(cursor);
    seig->Per_Sample_IV_Size = batom_cursor_byte(cursor);
    for(i=0;i<16;i++) {
        seig->KID[i] = batom_cursor_byte(cursor);
    }
    if(BATOM_IS_EOF(cursor)) {
        return false;
    }
    seig->constant_IV_size = 0;
    if(seig->isProtected == 1 && seig->Per_Sample_IV_Size==0) {
        seig->constant_IV_size = batom_cursor_byte(cursor);
        if(BATOM_IS_EOF(cursor)) {
            return false;
        }
        if(seig->constant_IV_size > sizeof(seig->constant_IV)/sizeof(seig->constant_IV[0])) {
            return false;
        }
        for(i=0;i<seig->constant_IV_size;i++) {
            seig->constant_IV[0] = batom_cursor_byte(cursor);
        }
    }
    BDBG_MSG(("bmp4_parse_CencSampleEncryptionInformationGroupEntry: crypt_byte_block:%u skip_byte_block:%u isProtected:%u Per_Sample_IV_Size:%u constant_IV_size:%u", seig->crypt_byte_block, seig->skip_byte_block, seig->isProtected, seig->Per_Sample_IV_Size, seig->constant_IV_size));
    return !BATOM_IS_EOF(cursor);
}

/* ISO/IEC 23001-7 3rd Edition  8.2   Track Encryption box */
bool bmp4_parse_TrackEncryption (batom_cursor *cursor, const bmp4_fullbox *box, struct bmp4_TrackEncryption *tenc)
{
    unsigned i;
    batom_cursor_skip(cursor,1);
    if(box->version==0) {
        tenc->default_crypt_skip_valid = false;
        tenc->default_crypt_byte_block = 0;
        tenc->default_skip_byte_block = 0;
        batom_cursor_skip(cursor,1);
    } else {
        uint8_t byte = batom_cursor_byte(cursor);
        tenc->default_crypt_skip_valid = true;
        tenc->default_crypt_byte_block = B_GET_BITS(byte, 8, 4);
        tenc->default_skip_byte_block = B_GET_BITS(byte, 4, 0);
    }
    tenc->default_isProtected = batom_cursor_byte(cursor);
    tenc->default_Per_Sample_IV_Size = batom_cursor_byte(cursor);
    for(i=0;i<16;i++) {
        tenc->default_KID[i] = batom_cursor_byte(cursor);
    }
    if(BATOM_IS_EOF(cursor)) {
        return false;
    }
    tenc->default_constant_IV_valid = false;
    if(tenc->default_isProtected && tenc->default_Per_Sample_IV_Size == 0) {
        tenc->default_constant_IV_size = batom_cursor_byte(cursor);
        if(BATOM_IS_EOF(cursor)) {
            return false;
        }
        if(tenc->default_constant_IV_size > sizeof(tenc->default_constant_IV)) {
            (void)BERR_TRACE(BERR_NOT_SUPPORTED);
            return false;
        }
        for(i=0;i<tenc->default_constant_IV_size;i++) {
            tenc->default_constant_IV[i] = batom_cursor_byte(cursor);
        }
    }
    BDBG_MSG(("TrackEncryption: version:%u default_skip_byte_block:%u default_crypt_byte_block:%u default_isProtected:%u default_Per_Sample_IV_Size:%u", box->version, tenc->default_crypt_byte_block, tenc->default_skip_byte_block, tenc->default_isProtected, tenc->default_Per_Sample_IV_Size));
    return !BATOM_IS_EOF(cursor);
}


/* ISO/IEC 23001-7 3rd Edition  7.2 Sample Encryption Information box for storage of sample auxiliary information  */
bool bmp4_parse_SampleEncryption(batom_cursor *cursor, struct bmp4_SampleEncryption *senc)
{
    senc->sample_count = batom_cursor_uint32_be(cursor);
    BDBG_MSG(("SampleEncryption: sample_count:%u", (unsigned)senc->sample_count));
    return !BATOM_IS_EOF(cursor);
}

bool bmp4_parse_SampleEncryptionEntry(batom_cursor *cursor, const bmp4_fullbox *box, unsigned Per_Sample_IV_Size, struct bmp4_SampleEncryptionEntry *entry)
{
    unsigned i;

    for(i=0;i<Per_Sample_IV_Size;i++) {
        entry->InitializationVector[i] = batom_cursor_byte(cursor);
    }
    if(box->flags & 0x02) {
        entry->subsample_count = batom_cursor_uint16_be(cursor);
    } else {
        entry->subsample_count = 0;
    }
    BDBG_MSG(("SampleEncryptionEntry: subsample_count:%u", (unsigned)entry->subsample_count));
    return !BATOM_IS_EOF(cursor);
}

bool bmp4_parse_SampleEncryptionSubsample(batom_cursor *cursor, struct bmp4_SampleEncryptionSubsample *subsample)
{
    subsample->BytesOfClearData = batom_cursor_uint16_be(cursor);
    subsample->BytesOfProtectedData = batom_cursor_uint32_be(cursor);
    BDBG_MSG(("SampleEncryptionSubsample: BytesOfClearData:%u BytesOfProtectedData:%u", (unsigned)subsample->BytesOfClearData, (unsigned)subsample->BytesOfProtectedData));
    return !BATOM_IS_EOF(cursor);
}

bool bmp4_parse_SchemeType(batom_cursor *cursor, const bmp4_fullbox *box, struct bmp4_SchemeType *schm)
{
    schm->scheme_type = batom_cursor_uint32_be(cursor);
    schm->scheme_version = batom_cursor_uint32_be(cursor);
    schm->scheme_uri_valid = false;
    if(box->flags & 0x01) {
        schm->scheme_uri_valid = true;
        if(!bmp4_parse_string(cursor, schm->scheme_uri, sizeof(schm->scheme_uri))) {
            return false;
        }
    }

    BDBG_MSG(("SchemeType: flags:%#x scheme_type:" B_MP4_TYPE_FORMAT " scheme_version:%u", (unsigned)box->flags, B_MP4_TYPE_ARG(schm->scheme_type), schm->scheme_version));
    return !BATOM_IS_EOF(cursor);
}

/* ISO/IEC 23001-7 3rd Edition  9.6  Pattern encryption */
unsigned
b_mp4_process_payload_cenc_subsample_cens_buffer_len(const struct b_mp4_cenc_frame *frame, unsigned crypt_byte_block, unsigned skip_byte_block)
{
    unsigned block_size = 16;
    unsigned full_block_len = (crypt_byte_block + skip_byte_block)*block_size;
    unsigned buf_len;
    unsigned crypt_bytes = crypt_byte_block * block_size;
    unsigned subsample;

    for(buf_len=0,subsample=0;subsample<frame->subsample_count;subsample++) {
        unsigned nvecs = frame->subsamples[subsample].count;
        const batom_vec *vecs = frame->vecs + frame->subsamples[subsample].offset;
        unsigned full_blocks;
        unsigned partial_block;
        unsigned buf_total_len;
        unsigned i;

        for(buf_total_len=0,i=0;i<nvecs;i++) {
            buf_total_len += vecs[i].len;
        }
        full_blocks = buf_total_len / full_block_len;
        partial_block = buf_total_len - full_blocks * full_block_len;
        if(partial_block > crypt_bytes) {
            partial_block = crypt_bytes;
        }

        buf_len += full_blocks * crypt_bytes + partial_block;
        BDBG_MSG(("b_mp4_process_payload_cenc_subsample_cens_buffer_len: subsample:%u buf_total_len:%u full_blocks:%u full_block_len:%u crypt_bytes:%u partial_block:%u buf_len:%u", subsample, buf_total_len, full_blocks, full_block_len, crypt_bytes, partial_block, buf_len));
    }
    return buf_len;
}

BERR_Code
b_mp4_process_payload_cenc_subsample_cens_buffer_iterate(const struct b_mp4_cenc_frame *frame, unsigned crypt_byte_block, unsigned skip_byte_block, void *context, BERR_Code (*apply)(void *, unsigned offset, void *, unsigned)  )
{
    unsigned block_size = 16;
    unsigned full_block_len = (crypt_byte_block + skip_byte_block)*block_size;
    unsigned buf_len;
    unsigned crypt_bytes = crypt_byte_block * block_size;
    unsigned subsample;
    unsigned buf_off=0;

    for(subsample=0;subsample<frame->subsample_count;subsample++) {
        unsigned nvecs = frame->subsamples[subsample].count;
        const batom_vec *vecs = frame->vecs + frame->subsamples[subsample].offset;
        unsigned prev_skip_bytes = 0;
        unsigned prev_crypt_bytes = 0;
        unsigned i;

        for(i=0;i<nvecs;i++) {
            unsigned len = vecs[i].len;
            unsigned vec_off = 0;
            BERR_Code rc;

            BDBG_MSG(("b_mp4_process_payload_cenc_subsample_cens_buffer_iterate: subsample:%u/%u buf_off:%u len:%u prev_crypt_bytes:%u prev_skip_bytes:%u", subsample, i, buf_off, len, prev_crypt_bytes, prev_skip_bytes));
            BDBG_ASSERT(prev_skip_bytes <= skip_byte_block * block_size);
            BDBG_ASSERT(prev_crypt_bytes < crypt_byte_block * block_size);
            if(prev_skip_bytes) {
                BDBG_ASSERT(prev_crypt_bytes==0);
                if(prev_skip_bytes >= len) {
                    prev_skip_bytes -= len;
                    continue;
                }
                vec_off = prev_skip_bytes;
                prev_skip_bytes = 0;
            } else if(prev_crypt_bytes) {
                if(prev_crypt_bytes >= len) {
                    rc = apply(context, buf_off, vecs[i].base, len);
                    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);}
                    buf_off += len;
                    prev_crypt_bytes -= len;
                    continue;
                } else {
                    rc = apply(context, buf_off, vecs[i].base, prev_crypt_bytes);
                    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);}
                    buf_off += prev_crypt_bytes;
                    vec_off += prev_crypt_bytes;
                    prev_skip_bytes = skip_byte_block*block_size;
                    prev_crypt_bytes = 0;
                    if(vec_off + prev_skip_bytes >= len) {
                        prev_skip_bytes -= len - vec_off;
                        continue;
                    }
                    vec_off += prev_skip_bytes;
                    prev_skip_bytes = 0;
                }
            }
            for(;;) {
                if(0) BDBG_LOG(("buf_off:%u buf_len:%u left:%u", buf_off, buf_len, len - vec_off));

                if(vec_off + crypt_bytes <= len ) {
                    rc = apply(context, buf_off, (uint8_t *)vecs[i].base + vec_off, crypt_bytes);
                    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);}
                    buf_off += crypt_bytes;
                    if(vec_off + full_block_len <= len) {
                        vec_off += full_block_len;
                    } else {
                        prev_skip_bytes = full_block_len - (len - vec_off);
                        break;
                    }
                } else {
                    unsigned left = len - vec_off;
                    if(0) BDBG_LOG(("left:%u len:%u vec_off:%u", left, len, vec_off));
                    if(left) {
                        BDBG_ASSERT(left < crypt_bytes);
                        rc = apply(context, buf_off, (uint8_t *)vecs[i].base + vec_off, left);
                        if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);}
                        buf_off += left;
                        prev_crypt_bytes = crypt_bytes - left;
                        BDBG_ASSERT(prev_crypt_bytes < crypt_byte_block * block_size);
                    } else {
                        prev_crypt_bytes = 0;
                    }
                    break;
                }
            }
        }
    }
    return BERR_SUCCESS;
}

BERR_Code
b_mp4_process_payload_cenc_subsample_accumulate(struct b_mp4_cenc_frame *frame, const batom_vec *vecs, unsigned nvecs)
{
    unsigned subsample;

    if(frame->vec_count + nvecs > B_MP4_CENC_MAX_FRAME_VECS) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    if(frame->subsample_count > B_MP4_CENC_MAX_FRAME_SUBSAMPLES ) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    subsample = frame->subsample_count;
    frame->subsample_count++;
    frame->subsamples[subsample].offset = frame->vec_count;
    frame->subsamples[subsample].count = nvecs;
    BKNI_Memcpy(frame->vecs + frame->vec_count, vecs, nvecs*sizeof(*vecs));
    frame->vec_count +=nvecs;
    return BERR_SUCCESS;
}
