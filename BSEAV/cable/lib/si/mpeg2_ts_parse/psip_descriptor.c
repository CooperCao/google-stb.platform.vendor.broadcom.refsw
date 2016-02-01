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
#include "psip_descriptor.h"
BDBG_MODULE(psip_descriptor);

uint8_t PSIP_CSD_getNumServices( TS_PSI_descriptor descriptor )
{
	CHECK( descriptor[0] == 0x86 );
	return (uint8_t)(descriptor[2] & 0x1F);
}

BERR_Code PSIP_CSD_getService( TS_PSI_descriptor descriptor, int serviceNum, PSIP_CSD_service *p_service )
{
	int byteOffset;

	CHECK( descriptor[0] == 0x86 );

	if( serviceNum >= (descriptor[2] & 0x1F) )
	{
		return BERR_INVALID_PARAMETER;
	}

	byteOffset = 3 + (6*serviceNum);

	p_service->language[0] = descriptor[byteOffset+0];
	p_service->language[1] = descriptor[byteOffset+1];
	p_service->language[2] = descriptor[byteOffset+2];
	p_service->cc_type = (descriptor[byteOffset+3]>>7)&1;
	if( p_service->cc_type )
	{
		p_service->cc.caption_service_number = (uint8_t)(descriptor[byteOffset+3]&0x3F);
	}
	else
	{
		p_service->cc.line21_field = descriptor[byteOffset+3]&1;
	}
	p_service->easy_reader = (descriptor[byteOffset+4]>>7)&1;
	p_service->wide_aspect_ratio = (descriptor[byteOffset+4]>>6)&1;

	return BERR_SUCCESS;
}

#define PSIP_CAD_RATING_DIM_SIZE (2*descriptor[byteOffset+1])	

static int PSIP_CAD_P_getRatingRegionByteOffset( TS_PSI_descriptor descriptor, int ratingRegionNum )
{
	uint8_t i;
	int byteOffset = 3;

	/* Jump to correct table (or first byte after last table) */
	for( i = 0; i < ratingRegionNum; i++ )
	{
		byteOffset += 2 + PSIP_CAD_RATING_DIM_SIZE + descriptor[byteOffset+2+PSIP_CAD_RATING_DIM_SIZE];

		CHECK( byteOffset < descriptor[1] );
	}

	return byteOffset;
}

uint8_t PSIP_CAD_getRatingRegionCount( TS_PSI_descriptor descriptor )
{
	CHECK( descriptor[0] == 0x87 );
	return (uint8_t)(descriptor[2] & 0x3F);
}

BERR_Code PSIP_CAD_getRatingRegion( TS_PSI_descriptor descriptor, int ratingRegionNum, PSIP_CAD_rating_region *p_ratingRegion )
{
	int byteOffset;
	CHECK( descriptor[0] == 0x87 );

	if( ratingRegionNum >= (descriptor[2] & 0x3F) )
	{
		return BERR_INVALID_PARAMETER;
	}

	byteOffset = PSIP_CAD_P_getRatingRegionByteOffset( descriptor, ratingRegionNum );
	p_ratingRegion->rating_region = descriptor[byteOffset+0];
	p_ratingRegion->rated_dimensions = descriptor[byteOffset+1];
	if( descriptor[byteOffset+2+PSIP_CAD_RATING_DIM_SIZE] )
	{
		p_ratingRegion->p_rating_description_text = &descriptor[byteOffset+3+PSIP_CAD_RATING_DIM_SIZE];
	}
	else
	{
		p_ratingRegion->p_rating_description_text = NULL;
	}

	return BERR_SUCCESS;
}

BERR_Code PSIP_CAD_getRatingDimension( TS_PSI_descriptor descriptor, int ratingRegionNum, int ratingDimensionNum, PSIP_CAD_rating_dimension *p_ratingDimension )
{
	int byteOffset;
	CHECK( descriptor[0] == 0x87 );

	if( ratingRegionNum >= (descriptor[2] & 0x3F) )
	{
		return BERR_INVALID_PARAMETER;
	}

	byteOffset = PSIP_CAD_P_getRatingRegionByteOffset( descriptor, ratingRegionNum );

	if( ratingRegionNum >= descriptor[byteOffset+1] )
	{
		return BERR_INVALID_PARAMETER;
	}

	p_ratingDimension->rating_dimension_j = descriptor[byteOffset+2+(2*ratingDimensionNum)];
	p_ratingDimension->rating_value = (uint8_t)(descriptor[byteOffset+2+(2*ratingDimensionNum)+1] & 0xF);

	return BERR_SUCCESS;
}

PSIP_MSS_string PSIP_ECND_getLongChannelName( TS_PSI_descriptor descriptor )
{
	CHECK( descriptor[0] == 0xA0 );
	return &descriptor[2];
}

void PSIP_SLD_getHeader( TS_PSI_descriptor descriptor, PSIP_SLD_header *p_header )
{
	CHECK( descriptor[0] == 0xA1 );

	p_header->PCR_PID = (uint16_t)(TS_READ_16( &descriptor[2] ) & 0x1FFF);
	p_header->number_elements = descriptor[4];
}

BERR_Code PSIP_SLD_getElement( TS_PSI_descriptor descriptor, int elementNum, PSIP_SLD_element *p_element )
{
	int byteOffset;

	CHECK( descriptor[0] == 0xA1 );
	if( elementNum >= descriptor[4] )
	{
		return BERR_INVALID_PARAMETER;
	}

	byteOffset = 5 + (elementNum*6);
	p_element->stream_type = descriptor[byteOffset+0];
	p_element->elementary_PID = (uint16_t)(TS_READ_16( &descriptor[byteOffset+1] ) & 0x1FFF);
	p_element->ISO_639_language_code[0] = descriptor[byteOffset+3];
	p_element->ISO_639_language_code[1] = descriptor[byteOffset+4];
	p_element->ISO_639_language_code[2] = descriptor[byteOffset+5];

	return BERR_SUCCESS;
}

uint8_t PSIP_TSSD_getNumServices( TS_PSI_descriptor descriptor )
{
	CHECK( descriptor[0] == 0xA2 );

	return (uint8_t)(descriptor[2] & 0x1F);
}

BERR_Code PSIP_TSSD_getService( TS_PSI_descriptor descriptor, int serviceNum, PSIP_TSSD_service *p_service )
{
	int byteOffset;

	CHECK( descriptor[0] == 0xA2 );
	if( serviceNum >= (descriptor[2] & 0x1F) )
	{
		return BERR_INVALID_PARAMETER;
	}

	byteOffset = 3 + (serviceNum*5);

	p_service->time_shift = (uint16_t)(TS_READ_16( &descriptor[byteOffset+0] ) & 0x3FF);
	p_service->major_channel_number = (uint16_t)((TS_READ_16( &descriptor[byteOffset+3] )>>2) & 0x3FF);
	p_service->minor_channel_number = (uint16_t)(TS_READ_16( &descriptor[byteOffset+4] ) & 0x3FF);

	return BERR_SUCCESS;
}

PSIP_MSS_string PSIP_CND_getComponentName( TS_PSI_descriptor descriptor )
{
	CHECK( descriptor[0] == 0xA3 );
	return &descriptor[2];
}

void PSIP_DCC_RD_getRequest( uint8_t *descriptor, PSIP_DCC_RD_request *p_request )
{
	CHECK( descriptor[0] == 0xA8 || descriptor[0] == 0xA9 );

	p_request->dcc_request_type = descriptor[2];
	p_request->p_dcc_request_text = &descriptor[4];
}

void PSIP_RCD_getRcInformation( TS_PSI_descriptor descriptor, uint8_t *p_numBytes, const uint8_t **pp_rc_information )
{
	CHECK( descriptor[0] == 0xAA );

	*p_numBytes = descriptor[1];
	*pp_rc_information = &descriptor[2];
}

uint8_t PSIP_GD_getNumGenres( TS_PSI_descriptor descriptor )
{
	CHECK( descriptor[0] == 0xA2 );

	return (uint8_t)(descriptor[2] & 0x1F);
}

BERR_Code PSIP_GD_getGenre( TS_PSI_descriptor descriptor, int genreNum, PSIP_GD_Genre *p_genre )
{
	int byteOffset;

	CHECK( descriptor[0] == 0xAB );
	if( genreNum >= (descriptor[2] & 0x1F) )
	{
		return BERR_INVALID_PARAMETER;
	}

	byteOffset = 3 + genreNum;
	*p_genre   = descriptor[byteOffset];

	return BERR_SUCCESS;
}
