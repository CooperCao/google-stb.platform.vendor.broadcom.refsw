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
