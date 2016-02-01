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


#ifndef SI_NTT_H
#define SI_NTT_H

#include "si_ntt_sns.h"
#define NTT_SNS_SUBTYPE		0x6

/* For the following, refer to table 5.10 of ANSI/SCTE65 2002 (DVS234) */
#define NTT_TABLE_ID_BYTE_INDX		0
#define NTT_TABLE_ID_BYTE_NUM			1
#define NTT_TABLE_ID_SHIFT			0
#define NTT_TABLE_ID_MASK				0xff

#define NTT_SECTION_LENGTH_BYTE_INDX    1
#define NTT_SECTION_LENGTH_BYTE_NUM    2
#define NTT_SECTION_LENGTH_SHIFT    0
#define NTT_SECTION_LENGTH_MASK    0x0fff

#define NTT_PROTOCOL_VERSION_BYTE_INDX    3
#define NTT_PROTOCOL_VERSION_BYTE_NUM    1
#define NTT_PROTOCOL_VERSION_SHIFT    0
#define NTT_PROTOCOL_VERSION_MASK    0x1f

#define NTT_ISO_639_CODE_BYTE_INDX    4
#define NTT_ISO_639_CODE_BYTE_NUM    3
#define NTT_ISO_639_CODE_SHIFT    0
#define NTT_ISO_639_CODE_MASK    0xffffff

#define NTT_TABLE_SUBTYPE_BYTE_INDX    7
#define NTT_TABLE_SUBTYPE_BYTE_NUM    1
#define NTT_TABLE_SUBTYPE_SHIFT    0
#define NTT_TABLE_SUBTYPE_MASK    0xf


#ifdef __cplusplus
extern "C" {
#endif

void SI_NTT_Init(void);
SI_RET_CODE SI_NTT_parse (unsigned char * table);
void SI_NTT_Get(unsigned char *p_NTT_SNS_version_number,
				unsigned long **p_NTT_SNS_section_mask);

#ifdef __cplusplus
}
#endif



#endif
