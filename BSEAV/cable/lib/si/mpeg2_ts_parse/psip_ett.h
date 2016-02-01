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
#ifndef PSIP_ETT_H__
#define PSIP_ETT_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	PSIP_ETT_channel_id,
	PSIP_ETT_event_id
} PSIP_ETT_id_type;

typedef struct
{
	struct
	{
		PSIP_ETT_id_type	id_type;
		uint16_t			source_id;
		uint16_t			event_id;
	} ETM_id;
	const uint8_t		*p_extended_text_message;
} PSIP_ETT_header;

void PSIP_ETT_getHeader( const uint8_t *buf, PSIP_ETT_header *p_header );

#ifdef __cplusplus
}
#endif
#endif
/* End of File */
