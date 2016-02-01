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
#include "bstd.h"
#include "ts_priv.h"

TS_PSI_descriptor TS_P_getDescriptor( const uint8_t *p_descBfr, uint32_t descriptorsLength, int descriptorNum )
{
	TS_PSI_descriptor descriptor;
	int i;
	uint32_t byteOffset = 0;

	descriptor = NULL;

	for( i = 0; byteOffset < descriptorsLength; i++ )
	{
		descriptor = &p_descBfr[byteOffset];
		byteOffset += p_descBfr[byteOffset+1] + 2;

		if( i == descriptorNum ) 
			break;
		else 
			descriptor = NULL;
	}

	return descriptor;
}
