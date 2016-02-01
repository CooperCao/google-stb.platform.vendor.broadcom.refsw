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

#ifndef SI_STT_H
#define SI_STT_H

/* For the following, refer to table 5.23 of ANSI/SCTE65 2002 (DVS234) */
#define STT_TABLE_ID_BYTE_INDX    0
#define STT_TABLE_ID_BYTE_NUM    1
#define STT_TABLE_ID_SHIFT    0
#define STT_TABLE_ID_MASK    0xff

#define STT_SECTION_LENGTH_BYTE_INDX    1
#define STT_SECTION_LENGTH_BYTE_NUM    2
#define STT_SECTION_LENGTH_SHIFT    0
#define STT_SECTION_LENGTH_MASK    0x0fff

#define STT_PROTOCOL_VERSION_BYTE_INDX    3
#define STT_PROTOCOL_VERSION_BYTE_NUM    1
#define STT_PROTOCOL_VERSION_SHIFT    0
#define STT_PROTOCOL_VERSION_MASK    0x1f

#define STT_SYSTEM_TIME_BYTE_INDX    5
#define STT_SYSTEM_TIME_BYTE_NUM    4
#define STT_SYSTEM_TIME_SHIFT    0
#define STT_SYSTEM_TIME_MASK    0xffffffff

#define STT_GPS_UTC_OFFSET_BYTE_INDX    9
#define STT_GPS_UTC_OFFSET_BYTE_NUM    1
#define STT_GPS_UTC_OFFSET_SHIFT    0
#define STT_GPS_UTC_OFFSET_MASK    0xff



#ifdef __cplusplus
extern "C" {
#endif

void SI_STT_Init (void (*cb));
void SI_STT_UnInit ();
SI_RET_CODE SI_STT_parse (unsigned char * stt_table);
unsigned long SI_STT_Get_Sys_Time (void);
unsigned long SI_STT_Get_GPS_UTC_Offset (void);
unsigned long SI_STT_Conv_To_UTC_Time (unsigned long time);

#ifdef __cplusplus
}
#endif



#endif
