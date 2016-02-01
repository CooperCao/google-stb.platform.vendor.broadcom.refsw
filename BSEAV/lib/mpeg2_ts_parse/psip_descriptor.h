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

#ifndef PSIP_DESCRIPTOR_H__
#define PSIP_DESCRIPTOR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "psip_common.h"

typedef struct
{
    uint8_t     descriptor_tag;
    uint8_t     descriptor_length;
} PSIP_descriptor_header;

/*

  Caption Service Descriptor (0x86)

 */

typedef struct
{
    uint8_t     language[3];
    bool        cc_type;
    union
    {
        bool    line21_field;
        uint8_t caption_service_number;
    } cc;
    bool        easy_reader;
    bool        wide_aspect_ratio;
} PSIP_CSD_service;

uint8_t PSIP_CSD_getNumServices( TS_PSI_descriptor descriptor );
BERR_Code PSIP_CSD_getService( TS_PSI_descriptor descriptor, int serviceNum, PSIP_CSD_service *p_service );

/*

  Content Advisory Descriptor (0x87)

 */

typedef struct
{
    uint8_t rating_region;
    uint8_t rated_dimensions;
    const uint8_t   *p_rating_description_text;
} PSIP_CAD_rating_region;

typedef struct
{
    uint8_t rating_dimension_j;
    uint8_t rating_value;
} PSIP_CAD_rating_dimension;

uint8_t PSIP_CAD_getRatingRegionCount( TS_PSI_descriptor descriptor );
BERR_Code PSIP_CAD_getRatingRegion( TS_PSI_descriptor descriptor, int ratingRegionNum, PSIP_CAD_rating_region *p_ratingRegion );
BERR_Code PSIP_CAD_getRatingDimension( TS_PSI_descriptor descriptor, int ratingRegionNum, int ratingDimensionNum, PSIP_CAD_rating_dimension *p_ratingDimension );

/*

  Extended Channel Name Descriptor (0xA0)

 */

PSIP_MSS_string PSIP_ECND_getLongChannelName( TS_PSI_descriptor descriptor );

/*

  Service Location Descriptor (0xA1)

 */

typedef struct
{
    uint16_t    PCR_PID;
    uint8_t     number_elements;
} PSIP_SLD_header;

typedef struct
{
    uint8_t     stream_type;
    uint16_t    elementary_PID;
    uint8_t     ISO_639_language_code[3];
} PSIP_SLD_element;

void PSIP_SLD_getHeader( TS_PSI_descriptor descriptor, PSIP_SLD_header *p_header );
BERR_Code PSIP_SLD_getElement( TS_PSI_descriptor descriptor, int elementNum, PSIP_SLD_element *p_element );

/*

  Time-Shifted Service Descriptor (0xA2)

 */

typedef struct
{
    uint16_t    time_shift;
    uint16_t    major_channel_number;
    uint16_t    minor_channel_number;
} PSIP_TSSD_service;

uint8_t PSIP_TSSD_getNumServices( TS_PSI_descriptor descriptor );
BERR_Code PSIP_TSSD_getService( TS_PSI_descriptor descriptor, int serviceNum, PSIP_TSSD_service *p_service );

/*

  Component Name Descriptor (0xA3)

 */

PSIP_MSS_string PSIP_CND_getComponentName( TS_PSI_descriptor descriptor );

/*

  DCC Departing Request Descriptor (0xA8) and  DCC Arriving Request Descriptor (0xA9)

 */

typedef enum
{
    PSIP_DCC_RD_cancel = 1,
    PSIP_DCC_RD_10_seconds,
    PSIP_DCC_RD_indefinite
} PSIP_DCC_RD_type;

typedef struct
{
    PSIP_DCC_RD_type    dcc_request_type;
    const uint8_t       *p_dcc_request_text;
} PSIP_DCC_RD_request;

void PSIP_DCC_RD_getRequest( uint8_t *descriptor, PSIP_DCC_RD_request *p_request );

/*

  Redistribution Control Descriptor (0xAA)

 */

void PSIP_RCD_getRcInformation( TS_PSI_descriptor descriptor, uint8_t *p_numBytes, const uint8_t **pp_rc_information );

/*

  Genre Descriptor (0xAB)

 */

typedef uint8_t PSIP_GD_Genre;

uint8_t PSIP_GD_getNumGenres( TS_PSI_descriptor descriptor );
BERR_Code PSIP_GD_getGenre( TS_PSI_descriptor descriptor, int genreNum, PSIP_GD_Genre *p_genre );

#ifdef __cplusplus
}
#endif
#endif
/* End of File */
