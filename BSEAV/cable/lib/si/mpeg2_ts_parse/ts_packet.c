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
#include "ts_packet.h"
#include "psip_priv.h"
BDBG_MODULE(ts_packet);

void TS_parseTsPacket( const uint8_t *buf, TS_packet *p_packet )
{
	CHECK( buf[0] == 0x47 );
	p_packet->transport_error_indicator = (buf[1]>>7)&1;
	p_packet->payload_unit_start_indicator = (buf[1]>>6)&1;
	p_packet->transport_priority = (buf[1]>>5)&1;
	p_packet->PID = (uint16_t)(TS_READ_16(&buf[1]) & 0x1FFF);
	p_packet->transport_scambling_control = (uint8_t)((buf[3]>>6)&3);
	p_packet->adaptation_field_control = (uint8_t)((buf[3]>>4)&3);
	p_packet->continuity_counter = (uint8_t)(buf[3]&0xF);

	if( p_packet->adaptation_field_control & 0x2 )
	{
		int byteOffset = 6;

		p_packet->adaptation_field.discontinuity_indicator				= (buf[5]>>7)&1;
		p_packet->adaptation_field.random_access_indicator				= (buf[5]>>6)&1;
		p_packet->adaptation_field.elementary_stream_priority_indicator	= (buf[5]>>5)&1;
		p_packet->adaptation_field.PCR_flag								= (buf[5]>>4)&1;
		p_packet->adaptation_field.OPCR_flag								= (buf[5]>>3)&1;
		p_packet->adaptation_field.splicing_point_flag					= (buf[5]>>2)&1;
		p_packet->adaptation_field.transport_private_data_flag			= (buf[5]>>1)&1;
		p_packet->adaptation_field.adaptation_field_extension_flag		= buf[5]&1;

		if( p_packet->adaptation_field.PCR_flag )
		{
			p_packet->adaptation_field.program_clock_reference_base = TS_READ_64( &buf[byteOffset] )>>31;
			p_packet->adaptation_field.program_clock_reference_extension = (uint16_t)(TS_READ_16( &buf[byteOffset+4] ) & 0x1FF);
			byteOffset += 6;
		}
		if( p_packet->adaptation_field.OPCR_flag )
		{
			p_packet->adaptation_field.original_program_clock_reference_base = TS_READ_64( &buf[byteOffset] )>>31;
			p_packet->adaptation_field.original_program_clock_reference_extension = (uint16_t)(TS_READ_16( &buf[byteOffset+4] ) & 0x1FF);
			byteOffset += 6;
		}
		if( p_packet->adaptation_field.splicing_point_flag )
		{
			p_packet->adaptation_field.splice_countdown = buf[byteOffset];
			byteOffset += 1;
		}
		if( p_packet->adaptation_field.transport_private_data_flag )
		{
			p_packet->adaptation_field.transport_private_data_length = buf[byteOffset];
			p_packet->adaptation_field.p_private_data_byte = &buf[byteOffset+1];
			byteOffset += 1 + p_packet->adaptation_field.transport_private_data_length;
		}
		if( p_packet->adaptation_field.adaptation_field_extension_flag )
		{
			byteOffset += 1;
			p_packet->adaptation_field.ltw_flag				= (buf[byteOffset]>>7)&1;
			p_packet->adaptation_field.piecewise_rate_flag	= (buf[byteOffset]>>6)&1;
			p_packet->adaptation_field.seamless_splice_flag	= (buf[byteOffset]>>5)&1;

			byteOffset += 1;
			if( p_packet->adaptation_field.ltw_flag )
			{
				p_packet->adaptation_field.ltw_valid_flag = (buf[byteOffset]>>7)&1;
				p_packet->adaptation_field.ltw_offset = (uint16_t)(TS_READ_16( &buf[byteOffset+1] ) & 0x7FFF);
				byteOffset += 2;
			}
			if( p_packet->adaptation_field.piecewise_rate_flag )
			{
				p_packet->adaptation_field.piecewise_rate = (TS_READ_32( &buf[byteOffset] )>>8) & 0x3FFFFF;
				byteOffset += 3;
			}
			if( p_packet->adaptation_field.seamless_splice_flag )
			{
				p_packet->adaptation_field.splice_type = (uint8_t)((buf[byteOffset]>>4)&0xF);
//				p_packet->adaptation_field.DTS_next_AU = ;
				byteOffset += 5;
			}
		}
	}

	if(p_packet->adaptation_field_control & 0x3)
	{
		if( p_packet->adaptation_field_control & 0x2 )
		{
			p_packet->p_data_byte = &buf[5+buf[4]];
			p_packet->data_size = (uint8_t)(188-(5+buf[4]));
		}
		else
		{
			p_packet->p_data_byte = &buf[4];
			p_packet->data_size = 188-4;
		}
	}
	else
	{
		p_packet->p_data_byte = NULL;
		p_packet->data_size = 0;
	}

}


