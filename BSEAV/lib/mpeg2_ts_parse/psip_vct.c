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
#include "psip_vct.h"
BDBG_MODULE(psip_vct);

static int PSIP_VCT_P_getChannelByteOffset( const uint8_t *buf, int channelNum )
{
    uint8_t i;
    int byteOffset = PSIP_TABLE_DATA_OFFSET + 1;

    if( channelNum == -1 )
    {
        channelNum = buf[PSIP_TABLE_DATA_OFFSET];
    }

    /* Jump to correct table (or first byte after last table) */
    for( i = 0; i < channelNum; i++ )
    {
        byteOffset += 32 + (TS_READ_16(&buf[byteOffset+30]) & 0x03FF);

        CHECK( byteOffset <= TS_PSI_MAX_BYTE_OFFSET(buf) );
    }

    return byteOffset;
}

uint8_t PSIP_VCT_getNumChannels( const uint8_t *buf )
{
    CHECK( (buf[0] == 0xC8 || buf[0] == 0xC9) );

    return buf[PSIP_TABLE_DATA_OFFSET];
}

TS_PSI_descriptor PSIP_VCT_getAdditionalDescriptor( const uint8_t *buf, int descriptorNum )
{
    int byteOffset;

    CHECK( (buf[0] == 0xC8 || buf[0] == 0xC9) );

    byteOffset = PSIP_VCT_P_getChannelByteOffset( buf, -1 );
    return TS_P_getDescriptor( &buf[byteOffset+2], (TS_READ_16(&buf[byteOffset]) & 0x03FF), descriptorNum );
}

BERR_Code PSIP_VCT_getChannel( const uint8_t *buf, int channelNum, PSIP_VCT_channel *p_channel )
{
    int byteOffset;
    int i;

    CHECK( (buf[0] == 0xC8 || buf[0] == 0xC9) );

    if( channelNum >= buf[PSIP_TABLE_DATA_OFFSET] )
    {
        return BERR_INVALID_PARAMETER;
    }

    byteOffset = PSIP_VCT_P_getChannelByteOffset( buf, channelNum );

    for( i = 0; i < 7; i++ )
    {
        p_channel->short_name[i] = TS_READ_16( &buf[byteOffset+(i*2)] );
    }

    p_channel->major_channel_number = (uint16_t)((TS_READ_16( &buf[byteOffset+14] ) >> 2) & 0x3FF);
    p_channel->minor_channel_number = (uint16_t)(TS_READ_16( &buf[byteOffset+15] ) & 0x3FF);
    p_channel->modulation_mode = buf[byteOffset+17];
    p_channel->carrier_frequency = TS_READ_32( &buf[byteOffset+18] );
    p_channel->channel_TSID = TS_READ_16( &buf[byteOffset+22] );
    p_channel->program_number = TS_READ_16( &buf[byteOffset+24] );
    p_channel->ETM_location = (buf[byteOffset+26]>>6)&3;
    p_channel->access_controlled = (uint8_t)((buf[byteOffset+26]>>5)&1);
    p_channel->hidden = (buf[byteOffset+26]>>4)&1;
    p_channel->path_select = (buf[byteOffset+26]>>3)&1;
    p_channel->out_of_band = (buf[byteOffset+26]>>2)&1;
    p_channel->hide_guide = (buf[byteOffset+26]>>1)&1;
    p_channel->service_type = buf[byteOffset+27]&0x3F;
    p_channel->source_id = TS_READ_16( &buf[byteOffset+28] );

    return BERR_SUCCESS;
}

TS_PSI_descriptor PSIP_VCT_getChannelDescriptor( const uint8_t *buf, int channelNum, int descriptorNum )
{
    int byteOffset;

    CHECK( (buf[0] == 0xC8 || buf[0] == 0xC9) );

    byteOffset = PSIP_VCT_P_getChannelByteOffset( buf, channelNum );
    return TS_P_getDescriptor( &buf[byteOffset+32], (TS_READ_16(&buf[byteOffset+30]) & 0x3FF), descriptorNum );
}
