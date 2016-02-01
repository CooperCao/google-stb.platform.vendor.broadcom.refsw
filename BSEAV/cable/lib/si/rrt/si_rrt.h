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

#ifndef SI_RRT_H
#define SI_RRT_H

typedef struct _SI_RRT_VALUE
{
	unsigned char abbrev_rating_value_length;
	char * abbrev_rating_value_text;
	unsigned char rating_value_length;
	char * rating_value_text;
} SI_RRT_VALUE;

typedef struct _SI_RRT_DIMENSION
{
	unsigned char dimension_name_length;
	char * dimension_name_text;
	unsigned char graduated_scale;
	unsigned char values_defined;
	SI_RRT_VALUE *rrt_values;
} SI_RRT_DIMENSION;

typedef struct _SI_RRT_REGION
{
	unsigned char rating_region;
	unsigned char version_number;
	unsigned char rating_region_name_length;
	char * rating_region_name_text;
	unsigned char dimensions_defined;
	SI_RRT_DIMENSION *rrt_dimensions;
	SI_LST_D_ENTRY(_SI_RRT_REGION) rrt_link;
} SI_RRT_REGION;

SI_LST_D_HEAD(rrt_region_list, _SI_RRT_REGION);

/* For the following, refer to table 5.31 of ANSI/SCTE65 2002 (DVS234) */
#define RRT_TABLE_ID_BYTE_INDX		0
#define RRT_TABLE_ID_BYTE_NUM			1
#define RRT_TABLE_ID_SHIFT			0
#define RRT_TABLE_ID_MASK				0xff

#define RRT_SECTION_LENGTH_BYTE_INDX    1
#define RRT_SECTION_LENGTH_BYTE_NUM    2
#define RRT_SECTION_LENGTH_SHIFT    0
#define RRT_SECTION_LENGTH_MASK    0x0fff

#define RRT_RATING_REGION_BYTE_INDX		4
#define RRT_RATING_REGION_BYTE_NUM			1
#define RRT_RATING_REGION_SHIFT			0
#define RRT_RATING_REGION_MASK				0xff

#define RRT_VERSION_NUMBER_BYTE_INDX    5
#define RRT_VERSION_NUMBER_BYTE_NUM    1
#define RRT_VERSION_NUMBER_SHIFT    1
#define RRT_VERSION_NUMBER_MASK    0x1f

#define RRT_CURRENT_NEXT_INDICATOR_BYTE_INDX    5
#define RRT_CURRENT_NEXT_INDICATOR_BYTE_NUM    1
#define RRT_CURRENT_NEXT_INDICATOR_SHIFT    0
#define RRT_CURRENT_NEXT_INDICATOR_MASK    0x01

#define RRT_SECTION_NUMBER_BYTE_INDX    6
#define RRT_SECTION_NUMBER_BYTE_NUM    1
#define RRT_SECTION_NUMBER_SHIFT    0
#define RRT_SECTION_NUMBER_MASK    0xff

#define RRT_LAST_SECTION_NUMBER_BYTE_INDX    7
#define RRT_LAST_SECTION_NUMBER_BYTE_NUM    1
#define RRT_LAST_SECTION_NUMBER_SHIFT    0
#define RRT_LAST_SECTION_NUMBER_MASK    0xff

#define RRT_PROTOCOL_VERSION_BYTE_INDX    8
#define RRT_PROTOCOL_VERSION_BYTE_NUM    1
#define RRT_PROTOCOL_VERSION_SHIFT    0
#define RRT_PROTOCOL_VERSION_MASK    0xff

#define RRT_REGION_NAME_LENGTH_BYTE_INDX    9
#define RRT_REGION_NAME_LENGTH_BYTE_NUM    1
#define RRT_REGION_NAME_LENGTH_SHIFT    0
#define RRT_REGION_NAME_LENGTH_MASK    0xff

/* define for dimensions_defined after region text. relative index offset. */
#define RRT_DIMENSIONS_DEFINED_BYTE_INDX    0
#define RRT_DIMENSIONS_DEFINED_BYTE_NUM    1
#define RRT_DIMENSIONS_DEFINED_SHIFT    0
#define RRT_DIMENSIONS_DEFINED_MASK    0xff

/* defines for dimension loop elements, relative offset only. */
#define RRT_DIMENSION_NAME_LENGTH_BYTE_INDX    0
#define RRT_DIMENSION_NAME_LENGTH_BYTE_NUM    1
#define RRT_DIMENSION_NAME_LENGTH_SHIFT    0
#define RRT_DIMENSION_NAME_LENGTH_MASK    0xff

/* defines for dimension loop elements after the name text. relative offset only. */
#define RRT_GRADUATED_SCALE_BYTE_INDX    0
#define RRT_GRADUATED_SCALE_BYTE_NUM    1
#define RRT_GRADUATED_SCALE_SHIFT    4
#define RRT_GRADUATED_SCALE_MASK    0x01

#define RRT_VALUES_DEFINED_BYTE_INDX    0
#define RRT_VALUES_DEFINED_BYTE_NUM    1
#define RRT_VALUES_DEFINED_SHIFT    0
#define RRT_VALUES_DEFINED_MASK    0x0f

/* defines for value loop elements, relative offset only. */
#define RRT_ABBREV_VALUE_LENGTH_BYTE_INDX    0
#define RRT_ABBREV_VALUE_LENGTH_BYTE_NUM    1
#define RRT_ABBREV_VALUE_LENGTH_SHIFT    0
#define RRT_ABBREV_VALUE_LENGTH_MASK    0xff

/* defines for value loop elements after abbrev_value_text, relative offset only. */
#define RRT_VALUE_LENGTH_BYTE_INDX    0
#define RRT_VALUE_LENGTH_BYTE_NUM    1
#define RRT_VALUE_LENGTH_SHIFT    0
#define RRT_VALUE_LENGTH_MASK    0xff

/* after the dimensions loop, relative offset. */
#define RRT_DESCRIPTORS_LENGTH_BYTE_INDX    0
#define RRT_DESCRIPTORS_LENGTH_BYTE_NUM    2
#define RRT_DESCRIPTORS_LENGTH_SHIFT    0
#define RRT_DESCRIPTORS_LENGTH_MASK    0x3ff


#ifdef __cplusplus
extern "C" {
#endif

void SI_RRT_Init(void);
SI_RET_CODE SI_RRT_parse (unsigned char * rrt_table);

#ifdef __cplusplus
}
#endif


#endif

