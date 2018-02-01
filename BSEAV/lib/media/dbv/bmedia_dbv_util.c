/***************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *******************************************************************************/
#include "bstd.h"
#include "bmp4_util.h"
#include "bmedia_dbv_util.h"

BDBG_MODULE(bmedia_dbv_util);

/* Dolby Vision Configuration Box and Decoder Configuration Record Confidential Information */

/* Dolby Vision Configuration Box and Decoder
 * Configuration Record
 * 3.1 Definition */
bool bmedia_parse_DOVIDecoderConfigurationRecord(batom_cursor *cursor, bmedia_DOVIDecoderConfigurationRecord *record)
{
    batom_bitstream bs;
    batom_cursor bs_cursor;
    bool result = true;
    size_t initial_position = batom_cursor_pos(cursor);
    size_t bs_position;

    record->dv_version_major =batom_cursor_byte(cursor);
    record->dv_version_minor =batom_cursor_byte(cursor);
    BATOM_CLONE(&bs_cursor, cursor);

    batom_bitstream_init(&bs, &bs_cursor);
    record->dv_profile=batom_bitstream_bits(&bs, 7);
    record->dv_level=batom_bitstream_bits(&bs, 6);
    record->rpu_present_flag=batom_bitstream_bit(&bs);
    record->el_present_flag=batom_bitstream_bit(&bs);
    record->bl_present_flag=batom_bitstream_bit(&bs);
    record->dv_bl_signal_compatibility_id=batom_bitstream_bits(&bs, 6);
    batom_bitstream_drop_bits(&bs, 28); /* const unsigned int (28) reserved = 0; */
    if(batom_bitstream_eof(&bs)) {
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
        goto eos;
    }
    bs_position = batom_bitstream_position(&bs);
    BDBG_ASSERT(bs_position>=initial_position);
    batom_cursor_skip(cursor, bs_position - initial_position);
    batom_cursor_skip(cursor, (32/8) * 4); /*  const unsigned int (32)[4] reserved = 0; */

done:
    BDBG_MSG(("DOVIDecoderConfigurationRecord:%p%s major:%u minor:%u provile:%u level:%u rpu_present_flag:%u el_present_flag:%u bl_present_flag:%u dv_bl_signal_compatibility_id:%u", (void*)cursor, result?"":" PARSING ERROR", (unsigned)record->dv_version_major, (unsigned)record->dv_version_minor, (unsigned)record->dv_profile, (unsigned)record->dv_level, (unsigned)record->rpu_present_flag, (unsigned)record->el_present_flag, (unsigned)record->bl_present_flag, (unsigned)record->dv_bl_signal_compatibility_id));
    return result;

eos:
    result = false;
    goto done;
}
