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


#ifndef SI_SVCT_H
#define SI_SVCT_H

#include "si_svct_vcm.h"
typedef enum
{
	VCM,
	DCM,
	ICM,
} SVCT_TABLE_SUBTYPE;

/* For the following, refer to table 5.13 of ANSI/SCTE65 2002 (DVS234) */
#define SVCT_TABLE_ID_BYTE_INDX		0
#define SVCT_TABLE_ID_BYTE_NUM			1
#define SVCT_TABLE_ID_SHIFT			0
#define SVCT_TABLE_ID_MASK				0xff

#define SVCT_SECTION_LENGTH_BYTE_INDX    1
#define SVCT_SECTION_LENGTH_BYTE_NUM    2
#define SVCT_SECTION_LENGTH_SHIFT    0
#define SVCT_SECTION_LENGTH_MASK    0x0fff

#define SVCT_PROTOCOL_VERSION_BYTE_INDX    3
#define SVCT_PROTOCOL_VERSION_BYTE_NUM    1
#define SVCT_PROTOCOL_VERSION_SHIFT    0
#define SVCT_PROTOCOL_VERSION_MASK    0x1f

#define SVCT_TRANS_MEDIUM_BYTE_INDX    4
#define SVCT_TRANS_MEDIUM_BYTE_NUM    1
#define SVCT_TRANS_MEDIUM_SHIFT    4
#define SVCT_TRANS_MEDIUM_MASK    0xf

#define SVCT_TABLE_SUBTYPE_BYTE_INDX    4
#define SVCT_TABLE_SUBTYPE_BYTE_NUM    1
#define SVCT_TABLE_SUBTYPE_SHIFT    0
#define SVCT_TABLE_SUBTYPE_MASK    0xf

#define SVCT_VCT_ID_BYTE_INDX    5
#define SVCT_VCT_ID_BYTE_NUM    2
#define SVCT_VCT_ID_SHIFT    0
#define SVCT_VCT_ID_MASK    0xffff


#ifdef __cplusplus
extern "C" {
#endif


void SI_SVCT_Init(void);
SI_RET_CODE SI_SVCT_parse (unsigned char * table);
unsigned long SI_SVCT_GetID();
void SI_SVCT_SetID(unsigned long vct_id);
void SI_SVCT_Get(unsigned char *p_SVCT_VCM_version_number,
				unsigned long **p_SVCT_VCM_section_mask
				);

#ifdef __cplusplus
}
#endif


#endif
