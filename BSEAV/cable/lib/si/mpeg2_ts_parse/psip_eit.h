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
#ifndef PSIP_EIT_H__
#define PSIP_EIT_H__

#include "psip_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	uint16_t			event_id;
	uint32_t			start_time;
	PSIP_ETM_location	ETM_location;
	uint32_t			length_in_seconds;
	const uint8_t		*p_title_text;
} PSIP_EIT_event;

uint8_t PSIP_EIT_getNumEvents( const uint8_t *buf );
BERR_Code PSIP_EIT_getEvent( const uint8_t *buf, int eventNum, PSIP_EIT_event *p_event );
TS_PSI_descriptor PSIP_EIT_getEventDescriptor( const uint8_t *buf, int eventNum, int descriptorNum );

#ifdef __cplusplus
}
#endif
#endif
/* End of File */
