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
#ifndef SI_AETT_H
#define SI_AETT_H

/* this file requires that si_list.h be included before it. */

/* For the following, refer to table 5.35 of ANSI/SCTE65 2002 (DVS234) */
typedef struct _SI_AETT_TEXT
{
	SI_LST_D_ENTRY(_SI_AETT_TEXT) text_link;
	unsigned short event_ID;
	unsigned short extended_text_length;
	char * extended_text_message;
} SI_AETT_TEXT;

typedef struct _SI_AETT_SOURCE
{
	SI_LST_D_ENTRY(_SI_AETT_SOURCE) source_link;
	unsigned short source_ID;
	SI_LST_D_HEAD(aett_text_list, _SI_AETT_TEXT) aett_text;
} SI_AETT_SOURCE;

typedef struct _SI_AETT_SLOT{
	SI_LST_D_ENTRY(_SI_AETT_SLOT) slot_link;
	unsigned char MGT_tag;
	unsigned char MGT_version_number;
	unsigned short pid;
	unsigned char version_number;
	unsigned long section_mask[8];
	SI_LST_D_HEAD(aett_source_list, _SI_AETT_SOURCE) aett_source;
} SI_AETT_SLOT;


/* For the following, refer to table 5.35 of ANSI/SCTE65 2002 (DVS234) */

#define AETT_TABLE_ID_BYTE_INDX		0
#define AETT_TABLE_ID_BYTE_NUM			1
#define AETT_TABLE_ID_SHIFT			0
#define AETT_TABLE_ID_MASK				0xff

#define AETT_SECTION_LENGTH_BYTE_INDX    1
#define AETT_SECTION_LENGTH_BYTE_NUM    2
#define AETT_SECTION_LENGTH_SHIFT    0
#define AETT_SECTION_LENGTH_MASK    0x0fff

#define AETT_AETT_SUBTYPE_BYTE_INDX    3
#define AETT_AETT_SUBTYPE_BYTE_NUM    1
#define AETT_AETT_SUBTYPE_SHIFT    0
#define AETT_AETT_SUBTYPE_MASK    0xff

#define AETT_MGT_TAG_BYTE_INDX    4
#define AETT_MGT_TAG_BYTE_NUM    1
#define AETT_MGT_TAG_SHIFT    0
#define AETT_MGT_TAG_MASK    0xff

#define AETT_VERSION_NUMBER_BYTE_INDX    5
#define AETT_VERSION_NUMBER_BYTE_NUM    1
#define AETT_VERSION_NUMBER_SHIFT    1
#define AETT_VERSION_NUMBER_MASK    0x1f

#define AETT_CURRENT_NEXT_INDICATOR_BYTE_INDX    5
#define AETT_CURRENT_NEXT_INDICATOR_BYTE_NUM    1
#define AETT_CURRENT_NEXT_INDICATOR_SHIFT    0
#define AETT_CURRENT_NEXT_INDICATOR_MASK    0x01

#define AETT_SECTION_NUMBER_BYTE_INDX    6
#define AETT_SECTION_NUMBER_BYTE_NUM    1
#define AETT_SECTION_NUMBER_SHIFT    0
#define AETT_SECTION_NUMBER_MASK    0xff

#define AETT_LAST_SECTION_NUMBER_BYTE_INDX    7
#define AETT_LAST_SECTION_NUMBER_BYTE_NUM    1
#define AETT_LAST_SECTION_NUMBER_SHIFT    0
#define AETT_LAST_SECTION_NUMBER_MASK    0xff

#define AETT_NUM_BLOCKS_IN_SECTION_BYTE_INDX    8
#define AETT_NUM_BLOCKS_IN_SECTION_BYTE_NUM    1
#define AETT_NUM_BLOCKS_IN_SECTION_SHIFT    0
#define AETT_NUM_BLOCKS_IN_SECTION_MASK    0xff

/* defines for items in the block loop relative offset only. */
#define AETT_ETM_ID_BYTE_INDX		0
#define AETT_ETM_ID_BYTE_NUM			4
#define AETT_ETM_ID_SHIFT			0
#define AETT_ETM_ID_MASK				0xffffffff

#define AETT_ETT_LENGTH_BYTE_INDX		4
#define AETT_ETT_LENGTH_BYTE_NUM			2
#define AETT_ETT_LENGTH_SHIFT			0
#define AETT_ETT_LENGTH_MASK				0xfff



#ifdef __cplusplus
extern "C" {
#endif

SI_AETT_SLOT *SI_AETT_Create_Slot (void);
SI_RET_CODE SI_AETT_Clear_Slot (SI_AETT_SLOT * slot);
SI_RET_CODE SI_AETT_Parse (unsigned char *aett_table);

#ifdef __cplusplus
}
#endif



#endif



