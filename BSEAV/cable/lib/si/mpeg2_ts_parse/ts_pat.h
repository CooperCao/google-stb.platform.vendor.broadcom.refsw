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
#ifndef TS_PAT_H__
#define TS_PAT_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	uint16_t	program_number;
	uint16_t	PID;
} TS_PAT_program;

/**
Returns true if it's a valid PAT.
The other TS_PAT functions assume you have validated the buffer beforehand.
**/
bool TS_PAT_validate(const uint8_t *buf, unsigned bfrSize);

uint8_t TS_PAT_getNumPrograms( const uint8_t *buf);
BERR_Code TS_PAT_getProgram( const uint8_t *buf, unsigned bfrSize, int programNum, TS_PAT_program *p_program );

#ifdef __cplusplus
}
#endif
#endif
/* End of File */
