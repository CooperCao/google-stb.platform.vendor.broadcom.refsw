/***************************************************************************
 *     Copyright (c) 2004-2005, Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ****************************************************************/
#include "tspsi_validate.h"

/* taken from BSEAV/lib/mpeg2_ts_parse/ts_priv.h */
#define TS_PSI_TABLE_ID_OFFSET              0
#define TS_PSI_SECTION_LENGTH_OFFSET        1
#define TS_PSI_TABLE_ID_EXT_OFFSET          3
#define TS_PSI_CNI_OFFSET                   5
#define TS_PSI_SECTION_NUMBER_OFFSET        6
#define TS_PSI_LAST_SECTION_NUMBER_OFFSET   7

#define TS_READ_16( buf ) ((uint16_t)((buf)[0]<<8|(buf)[1]))
#define TS_PSI_GET_SECTION_LENGTH( buf )    (uint16_t)(TS_READ_16( &(buf)[TS_PSI_SECTION_LENGTH_OFFSET] ) & 0x0FFF)
#define TS_PSI_MAX_BYTE_OFFSET( buf )       (TS_PSI_GET_SECTION_LENGTH(buf) - 1)

bool TS_Filter_Compare(const uint8_t *msg, const uint8_t *inclMask, const uint8_t *exclMask,
	const uint8_t *coef, size_t filterSize )
{
	bool inclResult = true;
	bool exclResult = true;
	bool exclEnabled = false;
	size_t i;

	for( i = 0; i < filterSize; i++ )
	{
		if( (inclMask[i] | coef[i]) != (inclMask[i] | msg[i]) )
		{
			inclResult = false;
			break;
		}

		if( exclEnabled == false && exclMask[i] != 0xFF )
		{
			exclEnabled = true;
			exclResult = false;
		}

		if( (~exclMask[i] & coef[i]) != (~exclMask[i] & msg[i]) )
		{
			exclResult = true;
		}
	}

#if 0
/* for debug to make sure the logic is right */
	if( !(inclResult && exclResult) )
	{
		printf("Incl: ");
		for( i = 0; i < filterSize; i++ )
		{
			printf("%02x ", inclMask[i]);
		}
		printf("\nExcl: ");
		for( i = 0; i < filterSize; i++ )
		{
			printf("%02x ", exclMask[i]);
		}
		printf("\nCoef: ");
		for( i = 0; i < filterSize; i++ )
		{
			printf("%02x ", coef[i]);
		}
		printf("\n Msg: ");
		for( i = 0; i < filterSize; i++ )
		{
			printf("%02x ", msg[i]);
		}
		printf("\n");
	}
#endif

	return inclResult && exclResult;
}

int TS_Get_Message_Size(const uint8_t *msg)
{
	size_t message_size = TS_PSI_GET_SECTION_LENGTH(msg) + 3;
	/* 
	The transport hardware automatically pads messages to be 32 bit aligned.
	Therefore, the message size must be increased to take this into account.
	*/
	if( message_size%4 ) {
		message_size += 4-(message_size%4);
	}
	return message_size;
}

bool TS_Validate_Size( const uint8_t *msg, size_t size)
{
	size_t current_size = 0;
	while (current_size + TS_PSI_SECTION_LENGTH_OFFSET + 1 < size) {
		current_size += TS_Get_Message_Size(&msg[current_size]);
	}
	/* In the end, it must be exactly equal */
	return current_size == size;
}

