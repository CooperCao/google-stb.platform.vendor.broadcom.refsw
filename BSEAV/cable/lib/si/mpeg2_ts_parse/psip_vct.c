/***************************************************************************
 *     Copyright (c) 2003, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
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
