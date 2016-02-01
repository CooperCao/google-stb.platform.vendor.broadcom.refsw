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

#ifndef SI_MGT_H
#define SI_MGT_H

SI_LST_D_HEAD(aeit_slot_list, _SI_AEIT_SLOT);
SI_LST_D_HEAD(aett_slot_list, _SI_AETT_SLOT);


/* For the following, refer to table 5.24 of ANSI/SCTE65 2002 (DVS234) */
/* for table header. */
#define MGT_TABLE_ID_BYTE_INDX		0
#define MGT_TABLE_ID_BYTE_NUM			1
#define MGT_TABLE_ID_SHIFT			0
#define MGT_TABLE_ID_MASK				0xff

#define MGT_SECTION_LENGTH_BYTE_INDX    1
#define MGT_SECTION_LENGTH_BYTE_NUM    2
#define MGT_SECTION_LENGTH_SHIFT    0
#define MGT_SECTION_LENGTH_MASK    0x0fff

#define MGT_VERSION_NUMBER_BYTE_INDX    5
#define MGT_VERSION_NUMBER_BYTE_NUM    1
#define MGT_VERSION_NUMBER_SHIFT    1
#define MGT_VERSION_NUMBER_MASK    0x1f

#define MGT_CURRENT_NEXT_INDICATOR_BYTE_INDX    5
#define MGT_CURRENT_NEXT_INDICATOR_BYTE_NUM    1
#define MGT_CURRENT_NEXT_INDICATOR_SHIFT    0
#define MGT_CURRENT_NEXT_INDICATOR_MASK    0x01

#define MGT_SECTION_NUMBER_BYTE_INDX    6
#define MGT_SECTION_NUMBER_BYTE_NUM    1
#define MGT_SECTION_NUMBER_SHIFT    0
#define MGT_SECTION_NUMBER_MASK    0xff

#define MGT_LAST_SECTION_NUMBER_BYTE_INDX    7
#define MGT_LAST_SECTION_NUMBER_BYTE_NUM    1
#define MGT_LAST_SECTION_NUMBER_SHIFT    0
#define MGT_LAST_SECTION_NUMBER_MASK    0xff

#define MGT_PROTOCOL_VERSION_BYTE_INDX    8
#define MGT_PROTOCOL_VERSION_BYTE_NUM    1
#define MGT_PROTOCOL_VERSION_SHIFT    0
#define MGT_PROTOCOL_VERSION_MASK    0xff

#define MGT_TABLES_DEFINED_BYTE_INDX    9
#define MGT_TABLES_DEFINED_BYTE_NUM    2
#define MGT_TABLES_DEFINED_SHIFT    0
#define MGT_TABLES_DEFINED_MASK    0xffff

/* for table type defines. */
#define MGT_TABLE_TYPE_BYTE_INDX    0
#define MGT_TABLE_TYPE_BYTE_NUM    2
#define MGT_TABLE_TYPE_SHIFT    0
#define MGT_TABLE_TYPE_MASK    0xffff

#define MGT_TABLE_TYPE_PID_BYTE_INDX    2
#define MGT_TABLE_TYPE_PID_BYTE_NUM    2
#define MGT_TABLE_TYPE_PID_SHIFT    0
#define MGT_TABLE_TYPE_PID_MASK    0x1fff

#define MGT_TABLE_TYPE_VERSION_NUMBER_BYTE_INDX    4
#define MGT_TABLE_TYPE_VERSION_NUMBER_BYTE_NUM    1
#define MGT_TABLE_TYPE_VERSION_NUMBER_SHIFT    0
#define MGT_TABLE_TYPE_VERSION_NUMBER_MASK    0x1f

#define MGT_NUMBER_BYTES_BYTE_INDX    5
#define MGT_NUMBER_BYTES_BYTE_NUM    4
#define MGT_NUMBER_BYTES_SHIFT    0
#define MGT_NUMBER_BYTES_MASK    0xffffffff

#define MGT_TABLE_TYPE_DESC_LENGTH_BYTE_INDX    9
#define MGT_TABLE_TYPE_DESC_LENGTH_BYTE_NUM    2
#define MGT_TABLE_TYPE_DESC_LENGTH_SHIFT    0
#define MGT_TABLE_TYPE_DESC_LENGTH_MASK    0x0fff

/* the MGT table descriptor start from the end of table types. */
#define MGT_DESC_LENGTH_BYTE_INDX    0
#define MGT_DESC_LENGTH_BYTE_NUM    2
#define MGT_DESC_LENGTH_SHIFT    0
#define MGT_DESC_LENGTH_MASK    0x0fff


/* MGT table type defines. refer to table 5.25 of ANSI/SCTE65 2002 (DVS234) */
#define MGT_TABLE_TYPE_LVCT_1		0x02
#define MGT_TABLE_TYPE_LVCT_0		0x03
#define MGT_TABLE_TYPE_SVCT_VCM		0x10
#define MGT_TABLE_TYPE_SVCT_DCM		0x11
#define MGT_TABLE_TYPE_SVCT_ICM		0x12
#define MGT_TABLE_TYPE_NIT_CDS		0x20
#define MGT_TABLE_TYPE_NIT_MMS		0x21
#define MGT_TABLE_TYPE_NTT_SNS		0x30
#define MGT_TABLE_TYPE_RRT_1			0x301
#define MGT_TABLE_TYPE_AEIT_START		0x1000
#define MGT_TABLE_TYPE_AEIT_END		0x10ff
#define MGT_TABLE_TYPE_AETT_START		0x1100
#define MGT_TABLE_TYPE_AETT_END		0x11ff



#ifdef __cplusplus
extern "C" {
#endif

void SI_MGT_Init (void);
SI_RET_CODE SI_MGT_parse (unsigned char * mgt_table);
SI_RET_CODE SI_MGT_AEIT_Del_All_Slot(void);
SI_RET_CODE SI_MGT_AETT_Del_All_Slot(void);
#ifdef __cplusplus
}
#endif



#endif

