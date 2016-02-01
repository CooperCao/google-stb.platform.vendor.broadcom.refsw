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
#include "psip_rrt.h"
BDBG_MODULE(psip_rrt);

#define RATING_REGION_OFFSET    (4)
#define VERSION_NUMBER_OFFSET   (5)
#define DIMENSION_BYTE_OFFSET   (PSIP_TABLE_DATA_OFFSET+buf[PSIP_TABLE_DATA_OFFSET]+1)
#define NUM_DIMENSIONS_DEFINED  (buf[DIMENSION_BYTE_OFFSET])
#define NUM_VALUES( p_dimBfr )  (uint8_t)((p_dimBfr)[(p_dimBfr)[0]+1]&0xF)


static int PSIP_RRT_P_getValueByteOffset( const uint8_t *p_dimBfr, int valueNum )
{
    int byteOffset;
    uint8_t i;

    if( valueNum == -1 )
    {
        valueNum = NUM_VALUES(p_dimBfr);
    }

    byteOffset = p_dimBfr[0]+2;

    for( i = 0; i < valueNum; i++ )
    {
        byteOffset += 2 + p_dimBfr[byteOffset] + p_dimBfr[1+byteOffset+p_dimBfr[byteOffset]];
    }

    return byteOffset;
}

static int PSIP_RRT_P_getDimensionByteOffset( const uint8_t *buf, int dimNum )
{
    int byteOffset;
    uint8_t i;

    if( dimNum == -1 )
    {
        dimNum = NUM_DIMENSIONS_DEFINED;
    }

    byteOffset = DIMENSION_BYTE_OFFSET+1;

    for( i = 0; i < dimNum; i++ )
    {
        byteOffset += PSIP_RRT_P_getValueByteOffset(&buf[byteOffset], -1 );
        CHECK( byteOffset <= TS_PSI_MAX_BYTE_OFFSET(buf) );
    }

    return byteOffset;
}

void PSIP_RRT_getHeader( const uint8_t *buf, PSIP_RRT_header *p_header )
{
    CHECK( buf[0] == 0xCA );

    p_header->rating_region             = buf[RATING_REGION_OFFSET];
    p_header->version_number            = ((buf[VERSION_NUMBER_OFFSET] >> 1) & (0 | (0x1F)));
    p_header->p_rating_region_name_text = &buf[PSIP_TABLE_DATA_OFFSET+1];
    p_header->dimensions_defined        = NUM_DIMENSIONS_DEFINED;
}

TS_PSI_descriptor PSIP_RRT_getDescriptor( const uint8_t *buf, int descriptorNum )
{
    int byteOffset;

    CHECK( buf[0] == 0xCA );

    byteOffset = PSIP_RRT_P_getDimensionByteOffset( buf, -1 );
    return TS_P_getDescriptor( &buf[byteOffset+2], (TS_READ_16(&buf[byteOffset]) & 0x3FF), descriptorNum );
}

BERR_Code PSIP_RRT_getDimension( const uint8_t *buf, int dimensionNum, PSIP_RRT_dimension *p_dimension )
{
    int byteOffset;

    CHECK( buf[0] == 0xCA );

    if( dimensionNum >= NUM_DIMENSIONS_DEFINED )
    {
        return BERR_INVALID_PARAMETER;
    }

    byteOffset = PSIP_RRT_P_getDimensionByteOffset( buf, dimensionNum );
    p_dimension->p_dimension_name_text = &buf[byteOffset+1];
    p_dimension->graduated_scale = (buf[byteOffset+buf[byteOffset]+1]>>4)&1;
    p_dimension->values_defined = (uint8_t)(buf[byteOffset+buf[byteOffset]+1]&0xF);

    return BERR_SUCCESS;
}

BERR_Code PSIP_RRT_getValue( const uint8_t *buf, int dimensionNum, int valueNum, PSIP_RRT_value *p_value )
{
    int byteOffset;

    CHECK( buf[0] == 0xCA );

    if( dimensionNum >= NUM_DIMENSIONS_DEFINED )
    {
        return BERR_INVALID_PARAMETER;
    }

    byteOffset = PSIP_RRT_P_getDimensionByteOffset( buf, dimensionNum );

    if( valueNum >= NUM_VALUES( &buf[byteOffset] ) )
    {
        return BERR_INVALID_PARAMETER;
    }

    byteOffset += PSIP_RRT_P_getValueByteOffset( &buf[byteOffset], valueNum );

    p_value->p_abbrev_rating_value_text = &buf[byteOffset+1];
    p_value->p_rating_value_text = &buf[byteOffset+1+buf[byteOffset]+1];

    return BERR_SUCCESS;
}
