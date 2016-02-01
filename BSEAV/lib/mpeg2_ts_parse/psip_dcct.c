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
#include "psip_priv.h"
#include "psip_dcct.h"
BDBG_MODULE(psip_dcct);

#define NUM_TERM_COUNT_OFFSET   14

static int PSIP_DCCT_P_getTermCountByteOffset( const uint8_t *p_dccTestBfr, int termCountNum )
{
    int byteOffset;

    uint8_t i;

    if( termCountNum == -1 )
    {
        termCountNum = p_dccTestBfr[NUM_TERM_COUNT_OFFSET];
    }

    byteOffset = NUM_TERM_COUNT_OFFSET+1;

    for( i = 0; i < termCountNum; i++ )
    {
        byteOffset += 11 + (TS_READ_16( &p_dccTestBfr[byteOffset+9] ) & 0x3FF);
    }

    return byteOffset;
}

static int PSIP_DCCT_P_getTestCountByteOffset( const uint8_t *buf, int testCountNum )
{
    int byteOffset;
    uint8_t i;

    if( testCountNum == -1 )
    {
        testCountNum = buf[PSIP_TABLE_DATA_OFFSET];
    }

    byteOffset = PSIP_TABLE_DATA_OFFSET+1;

    for( i = 0; i < testCountNum; i++ )
    {
        byteOffset += PSIP_DCCT_P_getTermCountByteOffset(&buf[byteOffset], -1 );
        byteOffset += (TS_READ_16( &buf[byteOffset] ) & 0x3FF) + 2;
        CHECK( byteOffset <= TS_PSI_MAX_BYTE_OFFSET(buf) );
    }

    return byteOffset;
}

void PSIP_DCCT_getHeader( const uint8_t *buf, PSIP_DCCT_header *p_header )
{
    p_header->dcc_subtype = buf[TS_PSI_TABLE_ID_EXT_OFFSET];
    p_header->dcc_id = buf[TS_PSI_TABLE_ID_EXT_OFFSET+1];
    p_header->dcc_test_count = buf[PSIP_TABLE_DATA_OFFSET];
}

TS_PSI_descriptor PSIP_DCCT_getAdditionalDescriptor( const uint8_t *buf, int descriptorNum )
{
    int byteOffset;

    byteOffset = PSIP_DCCT_P_getTestCountByteOffset( buf, -1 );
    return TS_P_getDescriptor( &buf[byteOffset+2], (TS_READ_16(&buf[byteOffset]) & 0x3FF), descriptorNum );
}

BERR_Code PSIP_DCCT_getTest( const uint8_t *buf, int testNum, PSIP_DCCT_test *p_test )
{
    int byteOffset;

    if( testNum >= buf[PSIP_TABLE_DATA_OFFSET] )
    {
        return BERR_INVALID_PARAMETER;
    }

    byteOffset = PSIP_DCCT_P_getTestCountByteOffset( buf, testNum );

    p_test->dcc_context = buf[byteOffset];
    p_test->dcc_from_major_channel_number = (uint16_t)((TS_READ_16(&buf[byteOffset])>>2)&0x3FF);
    p_test->dcc_from_minor_channel_number = (uint16_t)(TS_READ_16(&buf[byteOffset+1])&0x3FF);
    p_test->dcc_to_major_channel_number = (uint16_t)((TS_READ_16(&buf[byteOffset+3])>>2)&0x3FF);
    p_test->dcc_to_minor_channel_number = (uint16_t)(TS_READ_16(&buf[byteOffset+4])&0x3FF);
    p_test->dcc_start_time = TS_READ_32(&buf[byteOffset+6]);
    p_test->dcc_end_time = TS_READ_32(&buf[byteOffset+10]);
    p_test->dcc_term_count = buf[byteOffset+14];

    return BERR_SUCCESS;
}

TS_PSI_descriptor PSIP_DCCT_getTestDescriptor( const uint8_t *buf, int testNum, int descriptorNum )
{
    int byteOffset;

    if( testNum >= buf[PSIP_TABLE_DATA_OFFSET] )
    {
        return NULL;
    }

    byteOffset = PSIP_DCCT_P_getTestCountByteOffset( buf, testNum );
    byteOffset += PSIP_DCCT_P_getTermCountByteOffset( &buf[byteOffset], -1 );
    return TS_P_getDescriptor( &buf[byteOffset+2], (TS_READ_16(&buf[byteOffset]) & 0x3FF), descriptorNum );
}

BERR_Code PSIP_DCCT_getTerm( const uint8_t *buf, int testNum, int termNum, PSIP_DCCT_term *p_term )
{
    int byteOffset;

    if( testNum >= buf[PSIP_TABLE_DATA_OFFSET] )
    {
        return BERR_INVALID_PARAMETER;
    }

    byteOffset = PSIP_DCCT_P_getTestCountByteOffset( buf, testNum );

    if( termNum >= buf[byteOffset+NUM_TERM_COUNT_OFFSET] )
    {
        return BERR_INVALID_PARAMETER;
    }

    byteOffset += PSIP_DCCT_P_getTermCountByteOffset( &buf[byteOffset], termNum );
    p_term->dcc_selection_type = buf[byteOffset];
    p_term->dcc_selection_id = TS_READ_64( &buf[byteOffset+1] );

    return BERR_SUCCESS;
}

TS_PSI_descriptor PSIP_DCCT_getTermDescriptor( const uint8_t *buf, int testNum, int termNum, int descriptorNum )
{
    int byteOffset;

    if( testNum >= buf[PSIP_TABLE_DATA_OFFSET] )
    {
        return NULL;
    }

    byteOffset = PSIP_DCCT_P_getTestCountByteOffset( buf, testNum );

    if( termNum >= buf[byteOffset+NUM_TERM_COUNT_OFFSET] )
    {
        return NULL;
    }

    byteOffset += PSIP_DCCT_P_getTermCountByteOffset( &buf[byteOffset], termNum );
    return TS_P_getDescriptor( &buf[byteOffset+11], (TS_READ_16(&buf[byteOffset+9]) & 0x3FF), descriptorNum );
}
