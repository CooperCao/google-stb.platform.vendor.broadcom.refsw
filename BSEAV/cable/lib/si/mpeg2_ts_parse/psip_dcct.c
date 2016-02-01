/***************************************************************************
 *     Copyright (c) 2003-2008, Broadcom Corporation
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
#include "psip_dcct.h"
BDBG_MODULE(psip_dcct);

#define NUM_TERM_COUNT_OFFSET	14

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
