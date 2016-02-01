/***************************************************************************
 *     Copyright (c) 2002-2009, Broadcom Corporation
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
 ***************************************************************************/

#ifndef SI_NIT_H
#define SI_NIT_H

#include "si_nit_cds.h"
#include "si_nit_mms.h"
/* For the following, refer to table 5.1 of ANSI/SCTE65 2002 (DVS234) */
#define NIT_TABLE_ID_BYTE_INDX		0
#define NIT_TABLE_ID_BYTE_NUM			1
#define NIT_TABLE_ID_SHIFT			0
#define NIT_TABLE_ID_MASK				0xff

#define NIT_SECTION_LENGTH_BYTE_INDX    1
#define NIT_SECTION_LENGTH_BYTE_NUM    2
#define NIT_SECTION_LENGTH_SHIFT    0
#define NIT_SECTION_LENGTH_MASK    0x0fff

#define NIT_PROTOCOL_VERSION_BYTE_INDX    3
#define NIT_PROTOCOL_VERSION_BYTE_NUM    1
#define NIT_PROTOCOL_VERSION_SHIFT    0
#define NIT_PROTOCOL_VERSION_MASK    0x1f

#define NIT_FIRST_INDEX_BYTE_INDX    4
#define NIT_FIRST_INDEX_BYTE_NUM    1
#define NIT_FIRST_INDEX_SHIFT    0
#define NIT_FIRST_INDEX_MASK    0xff

#define NIT_NUMBER_OF_RECORDS_BYTE_INDX    5
#define NIT_NUMBER_OF_RECORDS_BYTE_NUM    1
#define NIT_NUMBER_OF_RECORDS_SHIFT    0
#define NIT_NUMBER_OF_RECORDS_MASK    0xff

#define NIT_TRANSMISSION_MEDIUM_BYTE_INDX    6
#define NIT_TRANSMISSION_MEDIUM_BYTE_NUM    1
#define NIT_TRANSMISSION_MEDIUM_SHIFT    4
#define NIT_TRANSMISSION_MEDIUM_MASK    0x0f

#define NIT_TABLE_SUBTYPE_BYTE_INDX    6
#define NIT_TABLE_SUBTYPE_BYTE_NUM    1
#define NIT_TABLE_SUBTYPE_SHIFT    0
#define NIT_TABLE_SUBTYPE_MASK    0x0f


/* For the following, refer to table 5.2 of ANSI/SCTE65 2002 (DVS234) */
#define 	CDS_SUBTYPE  1
#define 	MMS_SUBTYPE  2


#ifdef __cplusplus
extern "C" {
#endif

void SI_NIT_Init (void);
SI_RET_CODE SI_NIT_parse (unsigned char * nit_table);
void SI_NIT_Get(unsigned char *p_NIT_Version_Number,
				unsigned long **p_NIT_Section_Mask
				);

#ifdef __cplusplus
}
#endif



#endif

