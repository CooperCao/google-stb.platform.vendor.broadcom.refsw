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
#include "psip_ett.h"

void PSIP_ETT_getHeader( const uint8_t *buf, PSIP_ETT_header *p_header )
{
	p_header->ETM_id.source_id = TS_READ_16( &buf[PSIP_TABLE_DATA_OFFSET] );
	p_header->ETM_id.event_id = (uint16_t)(TS_READ_16( &buf[PSIP_TABLE_DATA_OFFSET+2] ) >> 2);
	p_header->ETM_id.id_type = ((buf[PSIP_TABLE_DATA_OFFSET+3]&0x2)?PSIP_ETT_event_id:PSIP_ETT_channel_id);
	p_header->p_extended_text_message = &buf[PSIP_TABLE_DATA_OFFSET+4];
}

