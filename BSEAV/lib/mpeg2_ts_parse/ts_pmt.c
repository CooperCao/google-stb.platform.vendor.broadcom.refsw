/******************************************************************************
 * (c) 2003-2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/

#include "bstd.h"
#include "ts_priv.h"
#include "ts_pmt.h"
BDBG_MODULE(ts_pmt);

#define PROGRAM_INFO_LENGTH_OFFSET (TS_PSI_LAST_SECTION_NUMBER_OFFSET+3)
#define PROGRAM_INFO_LENGTH(buf) (TS_READ_16(&buf[PROGRAM_INFO_LENGTH_OFFSET])&0xFFF)
#define DESCRIPTOR_BASE(buf) (&buf[TS_PSI_LAST_SECTION_NUMBER_OFFSET+5])
#define STREAM_BASE(buf) (TS_PSI_LAST_SECTION_NUMBER_OFFSET + 5 + PROGRAM_INFO_LENGTH(buf))

bool TS_PMT_validate(const uint8_t *buf, unsigned bfrSize)
{
    int sectionEnd = TS_PSI_GET_SECTION_LENGTH(buf) + TS_PSI_SECTION_LENGTH_OFFSET;
    int programInfoEnd = PROGRAM_INFO_LENGTH(buf) + PROGRAM_INFO_LENGTH_OFFSET;

    return (buf[0] == 0x2) &&
        sectionEnd < (int)bfrSize &&
        programInfoEnd < (int)bfrSize &&
        programInfoEnd < sectionEnd;
}

static int TS_PMT_P_getStreamByteOffset( const uint8_t *buf, unsigned bfrSize, int streamNum )
{
    int byteOffset;
    int i;

    /* After the last descriptor */
    byteOffset = STREAM_BASE(buf);

    for (i=0; i < streamNum; i++)
    {
        if (byteOffset >= (int)bfrSize || byteOffset >= TS_PSI_MAX_BYTE_OFFSET(buf))
            return -1;
        byteOffset += 5 + (TS_READ_16( &buf[byteOffset+3] ) & 0xFFF);
    }

    return byteOffset;
}


uint16_t TS_PMT_getPcrPid( const uint8_t *buf, unsigned bfrSize)
{
    BSTD_UNUSED(bfrSize);
    return (uint16_t)(TS_READ_16( &buf[TS_PSI_LAST_SECTION_NUMBER_OFFSET+1] ) & 0x1FFF);
}

TS_PSI_descriptor TS_PMT_getDescriptor( const uint8_t *buf, unsigned bfrSize, int descriptorNum )
{
    const uint8_t *descriptorBase = DESCRIPTOR_BASE(buf);
    uint32_t descriptorsLength = PROGRAM_INFO_LENGTH(buf);

    /* Any time we dereference memory based on the contents of live data,
    we should check. */
    if (descriptorBase - buf >= (int)bfrSize) {
        return NULL;
    }
    bfrSize -= (descriptorBase - buf);
    if (bfrSize < descriptorsLength) {
        BDBG_WRN(("Invalid descriptor length: %d>=%d", descriptorsLength, bfrSize));
        return NULL;
    }
    return TS_P_getDescriptor(descriptorBase, descriptorsLength, descriptorNum);
}

int TS_PMT_getNumStreams( const uint8_t *buf, unsigned bfrSize)
{
    int byteOffset;
    int i = 0;

    byteOffset = STREAM_BASE(buf);

    while (byteOffset < TS_PSI_MAX_BYTE_OFFSET(buf) && byteOffset < (int)bfrSize)
    {
        byteOffset += 5 + (TS_READ_16( &buf[byteOffset+3] ) & 0xFFF);
        i++;
    }

    return i;
}

BERR_Code TS_PMT_getStream( const uint8_t *buf, unsigned bfrSize, int streamNum, TS_PMT_stream *p_stream )
{
    int byteOffset;

    byteOffset = TS_PMT_P_getStreamByteOffset( buf, bfrSize, streamNum );
    if (byteOffset == -1)
        return BERR_INVALID_PARAMETER;

    p_stream->stream_type = buf[byteOffset];
    p_stream->elementary_PID = (uint16_t)(TS_READ_16( &buf[byteOffset+1] ) & 0x1FFF);

    return BERR_SUCCESS;
}

TS_PSI_descriptor TS_PMT_getStreamDescriptor( const uint8_t *buf, unsigned bfrSize, int streamNum, int descriptorNum )
{
    int byteOffset;

    byteOffset = TS_PMT_P_getStreamByteOffset( buf, bfrSize, streamNum );
    if (byteOffset == -1)
        return NULL;

    return (TS_P_getDescriptor(&buf[byteOffset+5], TS_READ_16(&buf[byteOffset+3])&0xFFF, descriptorNum ));
}
