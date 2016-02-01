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
#ifndef PSIP_DCCSCT_H__
#define PSIP_DCCSCT_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	uint16_t	dccsct_type;
	uint8_t		updates_defined;
} PSIP_DCCSCT_header;

typedef enum
{
    PSIP_DCCSCT_reserved,
	PSIP_DCCSCT_new_genre_category,
	PSIP_DCCSCT_new_state,
	PSIP_DCCSCT_new_county
} PSIP_DCCSCT_update_type;

typedef struct
{
	uint8_t			genre_category_code;
	const uint8_t	*p_genre_category_name_text;
} PSIP_DCCSCT_genre;

typedef struct
{
	uint8_t			dcc_state_location_code;
	const uint8_t	*p_dcc_state_location_code_text;
} PSIP_DCCSCT_state;

typedef struct
{
	uint8_t			state_code;
	uint16_t		dcc_county_location_code;
	const uint8_t	*p_dcc_county_location_code_text;
} PSIP_DCCSCT_county;

typedef struct
{
	PSIP_DCCSCT_update_type	update_type;
	union
	{
		PSIP_DCCSCT_genre	genre;
		PSIP_DCCSCT_state	state;
		PSIP_DCCSCT_county	county;
	} update;
} PSIP_DCCSCT_update;

void PSIP_DCCSCT_getHeader( const uint8_t *buf, PSIP_DCCSCT_header *p_header );
TS_PSI_descriptor PSIP_DCCSCT_getAdditionalDescriptor( const uint8_t *buf, int descriptorNum );

BERR_Code PSIP_DCCT_getUpdate( const uint8_t *buf, int updateNum, PSIP_DCCSCT_update *p_update );
TS_PSI_descriptor PSIP_DCCT_getUpdateDescriptor( const uint8_t *buf, int updateNum, int descriptorNum );

#ifdef __cplusplus
}
#endif
#endif
/* End of File */
