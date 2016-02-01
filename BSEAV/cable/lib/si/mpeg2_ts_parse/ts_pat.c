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
#include "ts_priv.h"
#include "ts_pat.h"
BDBG_MODULE(ts_pat);

bool TS_PAT_validate(const uint8_t *buf, unsigned bfrSize)
{
	BSTD_UNUSED(bfrSize);
	return (buf[0] == 0x00);
}

BERR_Code TS_PAT_getProgram( const uint8_t *buf, unsigned bfrSize, int programNum, TS_PAT_program *p_program )
{
	int byteOffset = TS_PSI_LAST_SECTION_NUMBER_OFFSET+1;

	byteOffset += programNum*4;

	if( byteOffset >= TS_PSI_MAX_BYTE_OFFSET(buf) || byteOffset >= (int)bfrSize)
	{
		return BERR_INVALID_PARAMETER;
	}

	p_program->program_number = TS_READ_16( &buf[byteOffset] );
	p_program->PID = (uint16_t)(TS_READ_16( &buf[byteOffset+2] ) & 0x1FFF);

	return BERR_SUCCESS;
}

uint8_t TS_PAT_getNumPrograms( const uint8_t *buf)
{
	return (TS_PSI_MAX_BYTE_OFFSET(buf)-(TS_PSI_LAST_SECTION_NUMBER_OFFSET+1))/4;
}
