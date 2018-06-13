/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
#define LOGE BDBG_ERR
#define LOGW BDBG_WRN
#define LOGD BDBG_MSG
#define LOGV BDBG_MSG

#include <string.h>
#include "cenc_util.h"
BDBG_MODULE(cenc_parser);
#include "dump_hex.h"

void cenc_parse_auxiliary_info_sizes(batom_cursor *cursor,
    bmp4_mp4_frag_headers *frag_header)
{
    uint32_t i;
    uint8_t version = 0;
    uint32_t flags = 0;
    uint8_t defsize = 0;
    uint32_t samplecount = 0;
    uint8_t sampleinfo = 0;

    LOGD(("%s: cursor.cursor=%p left=%d pos=%u count=%u", BSTD_FUNCTION, cursor->cursor, cursor->left, cursor->pos, cursor->count));
    batom_cursor_copy(cursor, &version, 1);
    if (version != 0) {
        LOGW(("%s: version %u not supported", BSTD_FUNCTION, version));
    }

    flags = batom_cursor_uint24_be(cursor);
    LOGD(("%s: flags 0x%x", BSTD_FUNCTION, flags));
    if (flags & 1) {
        /* TODO: Complete handling of aux_info_type and parameter */
        /* For now, simply move the cursor to the correct location */
        batom_cursor_uint32_be(cursor);
        batom_cursor_uint32_be(cursor);
    }

    batom_cursor_copy(cursor, &defsize, 1);
    LOGD(("%s: defsize %u", BSTD_FUNCTION, defsize));
    frag_header->samples_info.default_sample_info_size = defsize;

    samplecount = batom_cursor_uint32_be(cursor);
    LOGD(("%s: sampleCount %u", BSTD_FUNCTION, samplecount));
    frag_header->samples_info.sample_count = samplecount;

    frag_header->aux_info_size = 0;
    if (defsize == 0) {
        for (i = 0; i < samplecount; i++) {
            batom_cursor_copy(cursor, &sampleinfo, 1);
            frag_header->samples_info.samples[i].size = sampleinfo;
            frag_header->aux_info_size += sampleinfo;
        }
    } else {
        frag_header->aux_info_size += samplecount * defsize;
    }
    LOGD(("%s: aux_info_size %u", BSTD_FUNCTION, (uint32_t)frag_header->aux_info_size));
}

uint32_t cenc_parse_auxiliary_info_offsets(batom_cursor *cursor,
    bmp4_mp4_frag_headers *frag_header)
{
    uint8_t version;
    uint32_t flags;
    uint32_t entrycount;
    uint32_t i;
    uint32_t drmoffset;

    LOGD(("%s: cursor.cursor=%p left=%d pos=%u count=%u", BSTD_FUNCTION, cursor->cursor, cursor->left, cursor->pos, cursor->count));
    batom_cursor_copy(cursor, &version, 1);
    LOGD(("%s: version %u", BSTD_FUNCTION, version));

    flags = batom_cursor_uint24_be(cursor);
    LOGD(("%s: flags 0x%x", BSTD_FUNCTION, flags));

    entrycount = batom_cursor_uint32_be(cursor);
    LOGD(("%s: entrycount %u", BSTD_FUNCTION, entrycount));

    for (i = 0; i < entrycount; i++) {
       frag_header->samples_info.samples[i].size = frag_header->sample_info[i].size;
        if (version == 0) {
            uint32_t tmp;
            tmp = batom_cursor_uint32_be(cursor);
            LOGD(("%s: offset[%d]=%lu", BSTD_FUNCTION, i, (long unsigned)tmp));
            frag_header->samples_info.samples[i].offset = tmp;
        } else {
            uint64_t tmp;
            tmp = batom_cursor_uint64_be(cursor);
            LOGD(("%s: offset[%d]=%llu", BSTD_FUNCTION, i, (long long unsigned)tmp));
            frag_header->samples_info.samples[i].offset = tmp;
        }
    }

    /* parse clear/encrypted data*/
    drmoffset = frag_header->samples_info.samples[0].offset;

    LOGD(("drmoffset=%u current moof size=%u", drmoffset,
        frag_header->current_moof_size));

    return drmoffset;
}

void cenc_parse_mdat_head(mp4_parse_frag_info *frag_info,
    bmp4_mp4_frag_headers *frag_header)
{
    uint32_t i, j;
    batom_cursor *cursor = &frag_info->cursor;
    uint32_t ivlength = 8;
    uint16_t numsubsamples;
    uint16_t numclear;
    uint32_t numencrypted;
    uint32_t length;

    LOGD(("%s: cursor.cursor=%p left=%d pos=%u count=%u", BSTD_FUNCTION,
        cursor->cursor, cursor->left, cursor->pos, cursor->count));

    LOGD(("sample_count %d", frag_header->samples_info.sample_count));
    /* read CencSampleAuxiliaryDataFormats */
    for (i = 0; i < frag_header->samples_info.sample_count; i++) {
        SampleInfo *smpl = &frag_header->samples_info.samples[i];
        smpl->size = frag_header->sample_info[i].size;

        memset(smpl->iv, 0, 16);
        length = batom_cursor_copy(cursor, smpl->iv, ivlength);
        dump_hex("mdat_head", (const char*)smpl->iv, ivlength, false);
        if (length != ivlength) {
            LOGE(("%s: could not read iv for %dth sample", BSTD_FUNCTION, i));
            return;
        }

        if (frag_header->samples_info.default_sample_info_size != 0) {
            /* Probably audio case */
            LOGD(("%s: only 1 subsample in %dth sample: size=%u", BSTD_FUNCTION,
                i, frag_info->sample_info[i].size));
            smpl->nbOfEntries = 1;
            smpl->entries[0].bytesOfClearData = 0;
            smpl->entries[0].bytesOfEncData = frag_info->sample_info[i].size;
            continue;
        }

        numsubsamples = batom_cursor_uint16_be(cursor);
        smpl->nbOfEntries = numsubsamples;

        for (j = 0; j < numsubsamples; j++) {
            numclear = batom_cursor_uint16_be(cursor);

            numencrypted = batom_cursor_uint32_be(cursor);
            smpl->entries[j].bytesOfClearData = numclear;
            smpl->entries[j].bytesOfEncData = numencrypted;
        }
    }
}

int cenc_parse_sample_encryption_box(
    batom_cursor *cursor,
    bmp4_mp4_frag_headers *frag_header)
{
    uint8_t version;
    uint32_t flags;
    uint32_t sample_count;
    uint32_t i;
    uint32_t ivlength = 8;
    uint32_t length;
    uint16_t subsample_count;
    uint16_t j;
    uint16_t bytes_clear;
    uint32_t bytes_encrypted;

    LOGD(("%s: cursor.cursor=%p left=%d pos=%u count=%u", BSTD_FUNCTION, cursor->cursor, cursor->left, cursor->pos, cursor->count));
    batom_cursor_copy(cursor, &version, 1);
    LOGD(("%s: version %u", BSTD_FUNCTION, version));

    flags = batom_cursor_uint24_be(cursor);
    LOGD(("%s: flags 0x%x", BSTD_FUNCTION, flags));

    sample_count = batom_cursor_uint32_be(cursor);
    LOGD(("%s: sample_count %u", BSTD_FUNCTION, sample_count));

    for (i = 0; i < sample_count; i++) {
        SampleInfo *smpl = &frag_header->samples_info.samples[i];
        smpl->size = frag_header->sample_info[i].size;

        memset(smpl->iv, 0, 16);
        length = batom_cursor_copy(cursor, smpl->iv, ivlength);
        dump_hex("encryption_box", (const char*)smpl->iv, ivlength, false);
        if (length != ivlength) {
            LOGE(("%s: could not read iv for %dth sample", BSTD_FUNCTION, i));
            return -1;
        }

        if (flags & 0x000002) {
            subsample_count = batom_cursor_uint16_be(cursor);
            smpl->nbOfEntries = subsample_count;

            for (j = 0; j < subsample_count; j++) {
                bytes_clear = batom_cursor_uint16_be(cursor);
                bytes_encrypted = batom_cursor_uint32_be(cursor);
                smpl->entries[j].bytesOfClearData = bytes_clear;
                smpl->entries[j].bytesOfEncData = bytes_encrypted;
            }
        }
    }
    return 0;
}
