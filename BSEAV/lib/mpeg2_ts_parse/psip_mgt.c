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
#include "psip_mgt.h"
BDBG_MODULE(psip_mgt);

static int PSIP_MGT_P_getTableByteOffset( const uint8_t *buf, int tableNum )
{
    uint16_t i;
    int byteOffset = PSIP_TABLE_DATA_OFFSET + 2;

    if( tableNum == -1 )
    {
        tableNum = TS_READ_16( &buf[PSIP_TABLE_DATA_OFFSET] );
    }

    /* Jump to correct table (or first byte after last table) */
    for( i = 0; i < tableNum; i++ )
    {
        byteOffset += 11 + (TS_READ_16(&buf[byteOffset+9]) & 0x0FFF);

        CHECK( byteOffset <= TS_PSI_MAX_BYTE_OFFSET(buf) );
    }

    return byteOffset;
}

uint16_t PSIP_MGT_getTablesDefined( const uint8_t *buf )
{
    CHECK( buf[0] == 0xC7 );
    return TS_READ_16( &buf[PSIP_TABLE_DATA_OFFSET] );
}

TS_PSI_descriptor PSIP_MGT_getAdditionalDescriptor( const uint8_t *buf, int descriptorNum )
{
    int byteOffset;

    CHECK( buf[0] == 0xC7 );

    byteOffset = PSIP_MGT_P_getTableByteOffset( buf, -1 );
    return TS_P_getDescriptor( &buf[byteOffset+2], (TS_READ_16(&buf[byteOffset]) & 0xFFF), descriptorNum );
}

BERR_Code PSIP_MGT_getTable( const uint8_t *buf, int tableNum, PSIP_MGT_table *p_table )
{
    int byteOffset;

    CHECK( buf[0] == 0xC7 );

    if( tableNum >= TS_READ_16( &buf[PSIP_TABLE_DATA_OFFSET] ) )
    {
        return BERR_INVALID_PARAMETER;
    }

    byteOffset = PSIP_MGT_P_getTableByteOffset( buf, tableNum );

    p_table->table_type = TS_READ_16( &buf[byteOffset] );
    p_table->table_type_PID = (uint16_t)(TS_READ_16( &buf[byteOffset+2] ) & 0x1FFF);
    p_table->table_type_version_number = (uint8_t)(buf[byteOffset+4] & 0x1F);
    p_table->number_bytes = TS_READ_32( &buf[byteOffset+5] );

    return BERR_SUCCESS;
}

TS_PSI_descriptor PSIP_MGT_getTableDescriptor( const uint8_t *buf, int tableNum, int descriptorNum )
{
    int byteOffset;

    CHECK( buf[0] == 0xC7 );

    byteOffset = PSIP_MGT_P_getTableByteOffset( buf, tableNum );
    return TS_P_getDescriptor( &buf[byteOffset+11], (TS_READ_16(&buf[byteOffset+9]) & 0xFFF), descriptorNum );
}
