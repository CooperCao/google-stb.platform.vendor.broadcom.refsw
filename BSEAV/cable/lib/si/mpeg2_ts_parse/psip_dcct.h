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
#ifndef PSIP_DCCT_H__
#define PSIP_DCCT_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	uint8_t		dcc_subtype;
	uint8_t		dcc_id;
	uint8_t		dcc_test_count;
} PSIP_DCCT_header;

typedef enum
{
	PSIP_DCCT_temporary_retune,
	PSIP_DCCT_channel_redirect
} PSIP_DCCT_context;

typedef struct
{
	PSIP_DCCT_context	dcc_context;
	uint16_t			dcc_from_major_channel_number;
	uint16_t			dcc_from_minor_channel_number;
	uint16_t			dcc_to_major_channel_number;
	uint16_t			dcc_to_minor_channel_number;
	uint32_t			dcc_start_time;
	uint32_t			dcc_end_time;
	uint32_t			dcc_term_count;
} PSIP_DCCT_test;

typedef enum
{
	PSIP_DCCT_all,
	PSIP_DCCT_north_west,
	PSIP_DCCT_north_central,
	PSIP_DCCT_north_east,
	PSIP_DCCT_west_central,
	PSIP_DCCT_central,
	PSIP_DCCT_east_central,
	PSIP_DCCT_south_west,
	PSIP_DCCT_south_central,
	PSIP_DCCT_south_east
} PSIP_DCCT_county_subdivision;

typedef struct
{
	uint8_t		dcc_selection_type;
	uint64_t	dcc_selection_id;
} PSIP_DCCT_term;

void PSIP_DCCT_getHeader( const uint8_t *buf, PSIP_DCCT_header *p_header );
TS_PSI_descriptor PSIP_DCCT_getAdditionalDescriptor( const uint8_t *buf, int descriptorNum );

BERR_Code PSIP_DCCT_getTest( const uint8_t *buf, int testNum, PSIP_DCCT_test *p_test );
TS_PSI_descriptor PSIP_DCCT_getTestDescriptor( const uint8_t *buf, int testNum, int descriptorNum );

BERR_Code PSIP_DCCT_getTerm( const uint8_t *buf, int testNum, int termNum, PSIP_DCCT_term *p_term );
TS_PSI_descriptor PSIP_DCCT_getTermDescriptor( const uint8_t *buf, int testNum, int termNum, int descriptorNum );

#ifdef __cplusplus
}
#endif
#endif
/* End of File */
