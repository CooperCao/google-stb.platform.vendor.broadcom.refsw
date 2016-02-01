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

#ifndef SI_AEIT_H
#define SI_AEIT_H

/* this file requires that si_list.h be included before it. */

/* For the following, refer to table 5.33 of ANSI/SCTE65 2002 (DVS234) */
typedef struct _SI_AEIT_EVENTS
{
	SI_LST_D_ENTRY(_SI_AEIT_EVENTS) event_link;
	unsigned short event_ID;
	unsigned long start_time;
	unsigned long duration;
	unsigned char title_length;
	char * title_text;
	unsigned char ETM_present;
} SI_AEIT_EVENT;


typedef struct _SI_AEIT_SOURCE
{
	SI_LST_D_ENTRY(_SI_AEIT_SOURCE) source_link;
	unsigned short source_ID;
	SI_LST_D_HEAD(aeit_event_list, _SI_AEIT_EVENTS) aeit_event;
} SI_AEIT_SOURCE;

typedef struct _SI_AEIT_SLOT{
	SI_LST_D_ENTRY(_SI_AEIT_SLOT) slot_link;
	unsigned char MGT_tag;
	unsigned char MGT_version_number;
	unsigned short pid;
	unsigned char version_number;
	unsigned long section_mask[8];
	unsigned long slot_start_time;
	unsigned long slot_end_time;
	SI_LST_D_HEAD(aeit_source_list, _SI_AEIT_SOURCE) aeit_source;
} SI_AEIT_SLOT;


/* For the following, refer to table 5.33 of ANSI/SCTE65 2002 (DVS234) */

#define AEIT_TABLE_ID_BYTE_INDX		0
#define AEIT_TABLE_ID_BYTE_NUM			1
#define AEIT_TABLE_ID_SHIFT			0
#define AEIT_TABLE_ID_MASK				0xff

#define AEIT_SECTION_LENGTH_BYTE_INDX    1
#define AEIT_SECTION_LENGTH_BYTE_NUM    2
#define AEIT_SECTION_LENGTH_SHIFT    0
#define AEIT_SECTION_LENGTH_MASK    0x0fff

#define AEIT_AEIT_SUBTYPE_BYTE_INDX    3
#define AEIT_AEIT_SUBTYPE_BYTE_NUM    1
#define AEIT_AEIT_SUBTYPE_SHIFT    0
#define AEIT_AEIT_SUBTYPE_MASK    0xff

#define AEIT_MGT_TAG_BYTE_INDX    4
#define AEIT_MGT_TAG_BYTE_NUM    1
#define AEIT_MGT_TAG_SHIFT    0
#define AEIT_MGT_TAG_MASK    0xff

#define AEIT_VERSION_NUMBER_BYTE_INDX    5
#define AEIT_VERSION_NUMBER_BYTE_NUM    1
#define AEIT_VERSION_NUMBER_SHIFT    1
#define AEIT_VERSION_NUMBER_MASK    0x1f

#define AEIT_CURRENT_NEXT_INDICATOR_BYTE_INDX    5
#define AEIT_CURRENT_NEXT_INDICATOR_BYTE_NUM    1
#define AEIT_CURRENT_NEXT_INDICATOR_SHIFT    0
#define AEIT_CURRENT_NEXT_INDICATOR_MASK    0x01

#define AEIT_SECTION_NUMBER_BYTE_INDX    6
#define AEIT_SECTION_NUMBER_BYTE_NUM    1
#define AEIT_SECTION_NUMBER_SHIFT    0
#define AEIT_SECTION_NUMBER_MASK    0xff

#define AEIT_LAST_SECTION_NUMBER_BYTE_INDX    7
#define AEIT_LAST_SECTION_NUMBER_BYTE_NUM    1
#define AEIT_LAST_SECTION_NUMBER_SHIFT    0
#define AEIT_LAST_SECTION_NUMBER_MASK    0xff

#define AEIT_NUM_SOURCES_IN_SECTION_BYTE_INDX    8
#define AEIT_NUM_SOURCES_IN_SECTION_BYTE_NUM    1
#define AEIT_NUM_SOURCES_IN_SECTION_SHIFT    0
#define AEIT_NUM_SOURCES_IN_SECTION_MASK    0xff

/* defines for items in the source loop relative offset only. */
#define AEIT_SOURCE_ID_BYTE_INDX    0
#define AEIT_SOURCE_ID_BYTE_NUM    2
#define AEIT_SOURCE_ID_SHIFT    0
#define AEIT_SOURCE_ID_MASK    0xffff

#define AEIT_NUM_EVENTS_BYTE_INDX    2
#define AEIT_NUM_EVENTS_BYTE_NUM    1
#define AEIT_NUM_EVENTS_SHIFT    0
#define AEIT_NUM_EVENTS_MASK    0xff

/* defines for items in the events loop relative offset only. */
#define AEIT_EVENT_ID_BYTE_INDX    0
#define AEIT_EVENT_ID_BYTE_NUM    2
#define AEIT_EVENT_ID_SHIFT    0
#define AEIT_EVENT_ID_MASK    0x3fff

#define AEIT_START_TIME_BYTE_INDX    2
#define AEIT_START_TIME_BYTE_NUM    4
#define AEIT_START_TIME_SHIFT    0
#define AEIT_START_TIME_MASK    0xffffffff

#define AEIT_ETM_PRESENT_BYTE_INDX    6
#define AEIT_ETM_PRESENT_BYTE_NUM    1
#define AEIT_ETM_PRESENT_SHIFT    4
#define AEIT_ETM_PRESENT_MASK    0x3

#define AEIT_DURATION_BYTE_INDX    6
#define AEIT_DURATION_BYTE_NUM    3
#define AEIT_DURATION_SHIFT    0
#define AEIT_DURATION_MASK    0xfffff

#define AEIT_TITLE_LENGTH_BYTE_INDX    9
#define AEIT_TITLE_LENGTH_BYTE_NUM    1
#define AEIT_TITLE_LENGTH_SHIFT    0
#define AEIT_TITLE_LENGTH_MASK    0xff


#define AEIT_DESC_LENGTH_BYTE_INDX    0
#define AEIT_DESC_LENGTH_BYTE_NUM    2
#define AEIT_DESC_LENGTH_SHIFT    0
#define AEIT_DESC_LENGTH_MASK    0x0fff



#ifdef __cplusplus
extern "C" {
#endif

SI_AEIT_SLOT *SI_AEIT_Create_Slot (void);
SI_RET_CODE SI_AEIT_Clear_Slot (SI_AEIT_SLOT * slot);
SI_RET_CODE SI_AEIT_Parse (unsigned char *aeit_table);

#ifdef __cplusplus
}
#endif



#endif


