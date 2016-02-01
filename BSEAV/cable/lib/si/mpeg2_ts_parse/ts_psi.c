/***************************************************************************
 *     Copyright (c) 2003-2010, Broadcom Corporation
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
#include "ts_priv.h"
#include "ts_psi.h"

void TS_PSI_getSectionHeader( const uint8_t *buf, TS_PSI_header *p_header )
{
	p_header->table_id = buf[TS_PSI_TABLE_ID_OFFSET];
	p_header->section_syntax_indicator = (buf[TS_PSI_SECTION_LENGTH_OFFSET]>>7)&1;
	p_header->private_indicator = (buf[TS_PSI_SECTION_LENGTH_OFFSET]>>6)&1;
	p_header->section_length = TS_PSI_GET_SECTION_LENGTH(buf);
	p_header->table_id_extension = (uint16_t)(TS_READ_16(&(buf)[TS_PSI_TABLE_ID_EXT_OFFSET] ) & 0xFFFF);
	p_header->version_number = (uint8_t)((buf[TS_PSI_CNI_OFFSET]>>1)&0x1F);
	p_header->current_next_indicator = buf[TS_PSI_CNI_OFFSET]&1;
	p_header->section_number = buf[TS_PSI_SECTION_NUMBER_OFFSET];
	p_header->last_section_number = buf[TS_PSI_LAST_SECTION_NUMBER_OFFSET];
	p_header->CRC_32 = TS_READ_32( &(buf[p_header->section_length+TS_PSI_SECTION_LENGTH_OFFSET-4]) );
}
