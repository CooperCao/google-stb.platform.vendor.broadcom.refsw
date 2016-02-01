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
#include "psip_rrt.h"
BDBG_MODULE(psip_rrt);

#define RATING_REGION_OFFSET	(4)
#define VERSION_NUMBER_OFFSET	(5)
#define DIMENSION_BYTE_OFFSET	(PSIP_TABLE_DATA_OFFSET+buf[PSIP_TABLE_DATA_OFFSET]+1)
#define NUM_DIMENSIONS_DEFINED	(buf[DIMENSION_BYTE_OFFSET])
#define NUM_VALUES( p_dimBfr )	(uint8_t)((p_dimBfr)[(p_dimBfr)[0]+1]&0xF)


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
