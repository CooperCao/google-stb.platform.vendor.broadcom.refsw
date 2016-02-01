
/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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
#ifndef TS_PRIV_H__
#define TS_PRIV_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ts_psi.h"

#define TS_READ_64( buf ) ((uint64_t)( \
	(uint64_t)(buf)[0]<<56| \
	(uint64_t)(buf)[1]<<48| \
	(uint64_t)(buf)[2]<<40| \
	(uint64_t)(buf)[3]<<32| \
	(uint32_t)((buf)[4]<<24|(buf)[5]<<16|(buf)[6]<<8|(buf)[7])) )
#define TS_READ_32( buf ) ((uint32_t)((buf)[0]<<24|(buf)[1]<<16|(buf)[2]<<8|(buf)[3]))
#define TS_READ_16( buf ) ((uint16_t)((buf)[0]<<8|(buf)[1]))

#define TS_PSI_TABLE_ID_OFFSET				0
#define TS_PSI_SECTION_LENGTH_OFFSET		1
#define TS_PSI_TABLE_ID_EXT_OFFSET			3
#define TS_PSI_CNI_OFFSET					5
#define TS_PSI_SECTION_NUMBER_OFFSET		6
#define TS_PSI_LAST_SECTION_NUMBER_OFFSET	7

#define TS_PSI_GET_SECTION_LENGTH( buf )	(uint16_t)(TS_READ_16( &(buf)[TS_PSI_SECTION_LENGTH_OFFSET] ) & 0x0FFF)
#define TS_PSI_MAX_BYTE_OFFSET( buf )		(TS_PSI_GET_SECTION_LENGTH(buf) - 1)

TS_PSI_descriptor TS_P_getDescriptor( const uint8_t *p_descBfr, uint32_t descriptorsLength, int descriptorNum );

#define CHECK(COND) \
	do {if (!(COND)) BDBG_ERR(("Bad CHECK: %s at %s, %d", #COND, __FILE__, __LINE__)); } while (0)

#ifdef __cplusplus
}
#endif
#endif
/* End of File */
