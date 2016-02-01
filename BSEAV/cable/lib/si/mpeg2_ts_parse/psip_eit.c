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
#include "psip_eit.h"
BDBG_MODULE(psip_eit);

#define TITLE_LENGTH_BYTE_OFFSET		(byteOffset+9)
#define DESCRIPTOR_LENGTH_BYTE_OFFSET	(TITLE_LENGTH_BYTE_OFFSET+1+buf[TITLE_LENGTH_BYTE_OFFSET])

static int PSIP_EIT_P_getEventByteOffset( const uint8_t *buf, int eventNum )
{
	uint8_t i;
	int byteOffset = PSIP_TABLE_DATA_OFFSET + 1;

	if( eventNum == -1 )
	{
		eventNum = buf[PSIP_TABLE_DATA_OFFSET];
	}

	/* Jump to correct table (or first byte after last table) */
	for( i = 0; i < eventNum; i++ )
	{
		byteOffset += 12 + buf[TITLE_LENGTH_BYTE_OFFSET] + (TS_READ_16(&buf[DESCRIPTOR_LENGTH_BYTE_OFFSET]) & 0x0FFF);

		CHECK( byteOffset <= TS_PSI_MAX_BYTE_OFFSET(buf) );
	}

	return byteOffset;
}


uint8_t PSIP_EIT_getNumEvents( const uint8_t *buf )
{
	return buf[PSIP_TABLE_DATA_OFFSET];
}

BERR_Code PSIP_EIT_getEvent( const uint8_t *buf, int eventNum, PSIP_EIT_event *p_event )
{
	int byteOffset;

	if( eventNum >= buf[PSIP_TABLE_DATA_OFFSET] )
	{
		return BERR_INVALID_PARAMETER;
	}

	byteOffset = PSIP_EIT_P_getEventByteOffset( buf, eventNum );

	p_event->event_id = (uint16_t)(TS_READ_16( &buf[byteOffset] ) & 0x3FFF);
	p_event->start_time = TS_READ_32( &buf[byteOffset+2] );
	p_event->ETM_location = (buf[byteOffset+6]>>4)&3;
	p_event->length_in_seconds = (TS_READ_32(&buf[byteOffset+6])>>8)&0x000FFFFF;
	p_event->p_title_text = &buf[TITLE_LENGTH_BYTE_OFFSET+1];

	return BERR_SUCCESS;
}

TS_PSI_descriptor PSIP_EIT_getEventDescriptor( const uint8_t *buf, int eventNum, int descriptorNum )
{
	int byteOffset;

	if( eventNum >= buf[PSIP_TABLE_DATA_OFFSET] )
	{
		return NULL;
	}

	byteOffset = PSIP_EIT_P_getEventByteOffset( buf, eventNum );

	return TS_P_getDescriptor( &buf[DESCRIPTOR_LENGTH_BYTE_OFFSET+2], (TS_READ_16(&buf[DESCRIPTOR_LENGTH_BYTE_OFFSET]) & 0xFFF), descriptorNum );
}
