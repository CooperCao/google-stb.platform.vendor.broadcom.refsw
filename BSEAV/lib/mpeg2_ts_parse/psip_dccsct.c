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
#include "psip_dccsct.h"
BDBG_MODULE(psip_dccsct);

static int PSIP_DCCSCT_P_getUpdateByteOffset( const uint8_t *buf, int updateNum )
{
    uint8_t i;
    int byteOffset = PSIP_TABLE_DATA_OFFSET + 1;

    if( updateNum == -1 )
    {
        updateNum = buf[PSIP_TABLE_DATA_OFFSET];
    }

    /* Jump to correct table (or first byte after last table) */
    for( i = 0; i < updateNum; i++ )
    {
        byteOffset += 2 + buf[byteOffset+1];
        byteOffset += 2 + (TS_READ_16(&buf[byteOffset])&0x3FF);

        CHECK( byteOffset <= TS_PSI_MAX_BYTE_OFFSET(buf) );
    }

    return byteOffset;
}


void PSIP_DCCSCT_getHeader( const uint8_t *buf, PSIP_DCCSCT_header *p_header )
{
    p_header->dccsct_type = TS_READ_16(&buf[TS_PSI_TABLE_ID_EXT_OFFSET]);
    p_header->updates_defined = buf[PSIP_TABLE_DATA_OFFSET];
}

TS_PSI_descriptor PSIP_DCCSCT_getAdditionalDescriptor( const uint8_t *buf, int descriptorNum )
{
    int byteOffset;

    byteOffset = PSIP_DCCSCT_P_getUpdateByteOffset( buf, -1 );
    return TS_P_getDescriptor( &buf[byteOffset+2], (TS_READ_16(&buf[byteOffset])&0x3FF), descriptorNum );
}

BERR_Code PSIP_DCCT_getUpdate( const uint8_t *buf, int updateNum, PSIP_DCCSCT_update *p_update )
{
    int byteOffset;

    if( updateNum >= buf[PSIP_TABLE_DATA_OFFSET] )
    {
        return BERR_INVALID_PARAMETER;
    }

    byteOffset = PSIP_DCCSCT_P_getUpdateByteOffset( buf, updateNum );
    p_update->update_type = buf[byteOffset];
    p_update->update.genre.genre_category_code = buf[byteOffset+2];

    switch( p_update->update_type )
    {
    case PSIP_DCCSCT_new_genre_category:
        /* Fallthrough */
    case PSIP_DCCSCT_new_state:
        p_update->update.genre.p_genre_category_name_text = &buf[byteOffset+3];
        break;
    case PSIP_DCCSCT_new_county:
        p_update->update.county.dcc_county_location_code = (uint16_t)(TS_READ_16(&buf[byteOffset+3]) & 0x3FF);
        p_update->update.county.p_dcc_county_location_code_text = &buf[byteOffset+5];
        break;
    default:
        CHECK(false);
        return BERR_INVALID_PARAMETER;
    }

    return BERR_SUCCESS;
}

TS_PSI_descriptor PSIP_DCCT_getUpdateDescriptor( const uint8_t *buf, int updateNum, int descriptorNum )
{
    int byteOffset;

    byteOffset = PSIP_DCCSCT_P_getUpdateByteOffset( buf, updateNum );
    byteOffset += 2 + buf[byteOffset+1];
    return TS_P_getDescriptor( &buf[byteOffset+2], (TS_READ_16(&buf[byteOffset])&0x3FF), descriptorNum );
}
