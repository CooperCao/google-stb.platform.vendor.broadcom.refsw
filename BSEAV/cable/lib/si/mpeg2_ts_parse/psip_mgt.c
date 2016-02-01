/***************************************************************************
 *     Copyright (c) 2003-2007, Broadcom Corporation
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
