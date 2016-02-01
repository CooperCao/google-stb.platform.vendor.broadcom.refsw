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
