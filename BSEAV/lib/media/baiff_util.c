/***************************************************************************
 * Copyright (C) 2009-2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 * BMedia library, AIFF stream
 *
 *******************************************************************************/
#include "bstd.h"
#include "baiff_util.h"

BDBG_MODULE(baiff_util);


bool 
baiff_read_comm(batom_cursor *cursor, baiff_comm *comm)
{
    uint8_t extended[10]; /* IEEE 80 bits  data type */
    uint64_t mantissa;
    int exponent;
    int i;

    comm->numChannels = batom_cursor_uint16_be(cursor);
    comm->numSampleFrames = batom_cursor_uint32_be(cursor);
    comm->sampleSize  = batom_cursor_uint16_be(cursor);
    batom_cursor_copy(cursor, extended, sizeof(extended));
    if(BATOM_IS_EOF(cursor)) {
        return false;
    }

    for(mantissa=0,i=0;i<8;i++) {
        mantissa = (mantissa<<8) | extended[i+2];
    }
    exponent = (((int)extended[0]&0x7F)<<8 ) |  extended[1];
    if(exponent == 0x7FFF && mantissa) {
        return false;
    }
    exponent -= 16383 + 63;      
    if (extended[0]&0x80) {
        mantissa = -mantissa;
    }
    if(exponent >= 64 || exponent <= -64) {
        return false;
    }
    if(exponent<0) {
        mantissa = mantissa>>(-exponent);
    } else {
        mantissa = mantissa<<exponent;
    }
    comm->sampleRate = mantissa;
    BDBG_MSG(("baiff_read_comm: numChannels:%u numSampleFrames:%u sampleSize:%u", (unsigned)comm->numChannels, (unsigned)comm->numSampleFrames, (unsigned)comm->sampleSize));
    return true;
}



