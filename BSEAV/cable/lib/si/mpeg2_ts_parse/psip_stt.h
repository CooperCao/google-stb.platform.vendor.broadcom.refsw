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
#ifndef PSIP_STT_H__
#define PSIP_STT_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	bool	DS_status;
	uint8_t	DS_day_of_month;
	uint8_t	DS_hour;
} PSIP_STT_daylight_savings;

typedef struct
{
	uint32_t					system_time;
	uint8_t						GPS_UTC_offset;
	PSIP_STT_daylight_savings	daylight_savings;
} PSIP_STT_header;

void PSIP_STT_getHeader( const uint8_t *buf, PSIP_STT_header *p_header );
TS_PSI_descriptor PSIP_STT_getDescriptor( const uint8_t *buf, int descriptorNum );

#ifdef __cplusplus
}
#endif
#endif
/* End of File */
