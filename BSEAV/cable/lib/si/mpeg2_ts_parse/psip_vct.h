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
#ifndef PSIP_VCT_H__
#define PSIP_VCT_H__

#include "psip_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	PSIP_VCT_analog,
	PSIP_VCT_SCTE_mode_1,
	PSIP_VCT_SCTE_mode_2,
	PSIP_VCT_ATSC_8,
	PSIP_VCT_ATSC_16
} PSIP_VCT_modulation_mode;

typedef enum
{
	PSIP_VCT_analog_television = 1,
	PSIP_VCT_ATSC_digital_television,
	PSIP_VCT_ATSC_audio,
	PSIP_VCT_ATSC_data_only_service
} PSIP_VCT_service_type;

typedef struct
{
	uint16_t					short_name[7];
	uint16_t					major_channel_number;
	uint16_t					minor_channel_number;
	PSIP_VCT_modulation_mode	modulation_mode;
	uint32_t					carrier_frequency;
	uint16_t					channel_TSID;
	uint16_t					program_number;
	PSIP_ETM_location			ETM_location;
	uint16_t					access_controlled;
	bool						hidden;
	bool						path_select;	/* valid for cable VCT only */
	bool						out_of_band;	/* valid for cable VCT only */
	bool						hide_guide;
	PSIP_VCT_service_type		service_type;
	uint16_t					source_id;
} PSIP_VCT_channel;

uint8_t PSIP_VCT_getNumChannels( const uint8_t *buf );
TS_PSI_descriptor PSIP_VCT_getAdditionalDescriptor( const uint8_t *buf, int descriptorNum );

BERR_Code PSIP_VCT_getChannel( const uint8_t *buf, int channelNum, PSIP_VCT_channel *p_channel );
TS_PSI_descriptor PSIP_VCT_getChannelDescriptor( const uint8_t *buf, int channelNum, int descriptorNum );

#ifdef __cplusplus
}
#endif
#endif
/* End of File */
